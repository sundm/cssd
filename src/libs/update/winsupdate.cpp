#include "winsupdate.h"
#include "JlCompress.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QProcess>

//#define ITRAC

#ifdef ITRAC
const QString sz_version_url("file/version/itrac");
const QString sz_update_url("file/updateItrac");
#else
const QString sz_version_url("file/version/ortrac");
const QString sz_update_url("file/updateOrtrac");
#endif // ITRAC



Winsupdate::Winsupdate(QString &path_url, QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	netManager = new QNetworkAccessManager(this);

	QUrl url;
	server_url = path_url;
 	url = QUrl(QString("%1/%2").arg(server_url).arg(sz_version_url));

	QNetworkRequest request;
	request.setUrl(url);

	QByteArray array("{}");
	request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

	//post
	mGetVerReply = netManager->post(request, array);
	connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

void Winsupdate::on_click_btn()
{
	QProcess *pro = new QProcess(this);

#ifdef ITRAC
#ifdef _DEBUG
	pro->startDetached("itracd.exe");
#else
	pro->startDetached("itrac.exe");
#endif // _DEBUG
#else
#ifdef _DEBUG
	pro->startDetached("ortracd.exe");
#else
	pro->startDetached("ortrac.exe");
#endif // _DEBUG
#endif // ITRAC


		
	qApp->quit();
}

void Winsupdate::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) 
{
	//qDebug() << "Total:  " << bytesTotal << "  current received : " << bytesReceived;
	ui.progressBar->setValue(bytesReceived);
}

void Winsupdate::httpDownloadFinished(QNetworkReply *reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		zipFile->waitForBytesWritten(5 * 1000);
		if (0 == zipFile->size())
		{
			ui.infoLabel->setText(QString::fromLocal8Bit("下载文件失败"));
			return;
		}

		QString md5 = getFileMd5(zipFile->fileName());
		if (0 == QString::compare(md5, zipFileMD5, Qt::CaseInsensitive))
		{
			JlCompress::extractDir(zipFile->fileName(), QDir::currentPath()); 
			ui.infoLabel->setText(QString::fromLocal8Bit("下载完成"));
		}
		else
			ui.infoLabel->setText(QString::fromLocal8Bit("文件md5校验失败"));

		

		zipFile->deleteLater();
		mDownloadReply->deleteLater();
		zipFile = Q_NULLPTR;
		mDownloadReply = Q_NULLPTR;
	}
	else
	{
		ui.infoLabel->setText(reply->errorString());
	}

}

void Winsupdate::replyFinished(QNetworkReply *reply)
{
	if (reply->error() == QNetworkReply::NoError)
	{
		QString res = reply->readAll();
		QJsonParseError parseJsonErr;
		QJsonDocument document = QJsonDocument::fromJson(res.toUtf8(), &parseJsonErr);
		if (!(parseJsonErr.error == QJsonParseError::NoError))
		{
			ui.infoLabel->setText(QString::fromLocal8Bit("解析json失败"));
			return;
		}
		QJsonObject jsonObject = document.object();
		zipFileName = jsonObject["version"].toString();
		zipFileMD5 = jsonObject["md5"].toString();

		ui.versionLabel->setText(QString::fromLocal8Bit("服务器最新版本: %1").arg(zipFileName));
		ui.MD5Label->setText(QString::fromLocal8Bit("MD5: %1").arg(zipFileMD5));
		ui.sizeLabel->setText(QString::fromLocal8Bit("文件大小: %1").arg(jsonObject["size"].toInt()));

		ui.progressBar->setMaximum(jsonObject["size"].toInt());

		if (!createFile())
		{
			ui.infoLabel->setText(QString::fromLocal8Bit("创建下载文件失败"));
			ui.pushButton->setEnabled(true);
			return;
		}

		QUrl url;
		url = QUrl(QString("%1/%2").arg(server_url).arg(sz_update_url));

		QNetworkRequest request;
		request.setUrl(url);

		//post
		mDownloadReply = netManager->get(request);
		disconnect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
		connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(httpDownloadFinished(QNetworkReply*)));
		connect(mDownloadReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
		connect(mDownloadReply, SIGNAL(readyRead()), this, SLOT(writeToFile()));

		mGetVerReply->deleteLater();
		mGetVerReply = Q_NULLPTR;
	}
	else
	{
		ui.infoLabel->setText(reply->errorString());
	}
}

void Winsupdate::writeToFile()
{
	QByteArray tem = mDownloadReply->readAll();

	zipFile->write(tem);
}

bool Winsupdate::createFile()
{
	QString fileName = QString("%1.zip").arg(zipFileName);
	QString currentPath = QDir::currentPath();
	QString filePath = QString("%1/update").arg(currentPath);
	QDir *folder = new QDir;
	if(!folder->exists(filePath))
	{
		if (!folder->mkdir(filePath)) {
			ui.infoLabel->setText(QString::fromLocal8Bit("创建文件夹失败"));
		}
	}

	QString zipFileName = QString("%1/%2").arg(filePath).arg(fileName);
	zipFile = new QFile(zipFileName);
	if (zipFile->exists())
		QFile::remove(zipFileName);

	if (!zipFile->open(QIODevice::WriteOnly))
		return false;

	return true;
}

QString Winsupdate::getFileMd5(QString filePath)
{
	QFile theFile(filePath);
	theFile.open(QIODevice::ReadOnly);
	QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
	theFile.close();
	return QString(ba.toHex().constData());
}