#include "winsupdate.h"
#include "ftpmanager.h"
#include "startupthread.h"
#include "../rdao/dao/verdao.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>

Winsupdate::Winsupdate(QWidget *parent)
	: QMainWindow(parent)
{
	isSuccess = false;
	ui.setupUi(this);

	ui.comboBox->addItem("itrac", 1);

	FtpManager::getInstance()->setHostPort("192.168.16.102");
	FtpManager::getInstance()->setUserInfo("ftp_user", "123456");

	connect(FtpManager::getInstance(), SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(uploadProgress(qint64, qint64)));
	connect(FtpManager::getInstance(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(uploadError(QNetworkReply::NetworkError)));
	connect(FtpManager::getInstance(), SIGNAL(uploadFinished()), this, SLOT(uploadFinished()));

	_thread = new StartupThread(this);
	connect(_thread, SIGNAL(message(const QString&, bool)), this, SLOT(showInfo(const QString&, bool)), Qt::QueuedConnection);
	connect(_thread, SIGNAL(finished()), this, SLOT(onThreadFinished()), Qt::QueuedConnection);

	_thread->start();
	
}

void Winsupdate::on_file_btn()
{
	QFileDialog *fileDialog = new QFileDialog(this);
	fileDialog->setWindowTitle(tr("选择更新程序"));
	fileDialog->setDirectory(".");
	fileDialog->setNameFilter(tr("exe(*.exe)"));
	fileDialog->setFileMode(QFileDialog::ExistingFiles);
	fileDialog->setViewMode(QFileDialog::Detail);

	QStringList fileNames;
	if (fileDialog->exec())
		fileNames = fileDialog->selectedFiles();

	if (fileNames.size() == 0 || fileNames.size() > 1) return;

	_filePath = fileNames.at(0);

	_fileMd5 = getFileMd5(_filePath);

	ui.MD5Label->setText(QString("MD5:").append(_fileMd5));
	ui.pathLabel->setText(QString("文件路径:").append(_filePath));
}

void Winsupdate::on_update_btn()
{
	if (_filePath.isEmpty() || _fileMd5.isEmpty())
	{
		QMessageBox::about(NULL, "提示", "请先选择更新文件");
		return;
	}

	int typeId = ui.comboBox->currentData().toInt();
	VerDao dao;
	result_t resp = dao.setVersion(typeId, "", _fileMd5);
	if (resp.isOk())
	{
		QString urlPath = "./update/update.exe";
		FtpManager::getInstance()->put(_filePath, urlPath);
	}
	else
	{
		QMessageBox::about(NULL, "提示", QString("插入数据库失败：%1").arg(resp.msg()));
		return;
	}
}

void Winsupdate::uploadError(QNetworkReply::NetworkError e)
{
	qDebug() << e;
}

void Winsupdate::uploadFinished()
{

}

void Winsupdate::uploadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	ui.progressBar->setMaximum(bytesTotal);
	ui.progressBar->setValue(bytesReceived);
}

QString Winsupdate::getFileMd5(QString filePath)
{
	QFile theFile(filePath);
	theFile.open(QIODevice::ReadOnly);
	QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
	theFile.close();
	return QString(ba.toHex().constData());
}