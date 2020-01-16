#include "addinstrumentdialog.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "xnotifier.h"
#include "ftpmanager.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include <xui/images.h>
#include "rdao/dao/InstrumentDao.h"
#include <qhttpmultipart.h>
#include <thirdparty/qjson/src/parser.h>
#include <QNetworkreply>
#include <QtWidgets/QtWidgets>

AddInstrumentDialog::AddInstrumentDialog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _pinyinEdit(new Ui::FlatEdit)
	, _checkVIPBox(new QCheckBox("贵重器械"))
	//, _checkImplantBox(new QCheckBox("植入器械"))
	, _imgLabel(new XPicture(this))
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("添加新器械");

	_isModify = false;

	_pinyinEdit->setInputValidator(Ui::InputValitor::LetterOnly);

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &AddInstrumentDialog::accept);

	_imgLabel->setFixedHeight(256);
	_imgLabel->setBgColor(QColor(245, 246, 247));
	_imgLabel->setHidden(true);

	QPushButton *loadButton = new QPushButton("加载图片");
	loadButton->setIcon(QIcon(":/res/plus-24.png"));
	connect(loadButton, SIGNAL(clicked()), this, SLOT(loadImg()));

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("器械名"), 0, 0);
	mainLayout->addWidget(new QLabel("拼音检索码"), 1, 0);
	
	mainLayout->addWidget(_nameEdit, 0, 1);
	mainLayout->addWidget(_pinyinEdit, 1, 1);
	mainLayout->addWidget(_checkVIPBox, 2, 0, 1, 1);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);

	mainLayout->addWidget(loadButton, 4, 0, 1, 2, Qt::AlignLeft);
	mainLayout->addWidget(_imgLabel, 5, 0, 1, 2);
	mainLayout->addWidget(submitButton, 6, 0, 1, 2, Qt::AlignHCenter);

	connect(FtpManager::getInstance(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(imgError(QNetworkReply::NetworkError)));
	connect(FtpManager::getInstance(), SIGNAL(uploadFinished()), this, SLOT(imgUploaded()));


	resize(parent ? parent->width() / 3 : 360, sizeHint().height());
}

void AddInstrumentDialog::setInfo(const QString &id)
{
	setWindowTitle("修改器械");
	_isModify = true;
	_instrumentId = id;

	InstrumentDao dao;
	InstrumentType it;
	result_t res = dao.getInstrumentType(_instrumentId.toInt(), &it);

	_nameEdit->setText(it.typeName);
	_pinyinEdit->setText(it.pinyin);
	_nameEdit->setReadOnly(_isModify);

	_checkVIPBox->setChecked(it.isVip);
	
	QString imgPath = QString("./photo/instrument/%1.jpg").arg(_instrumentId);
	QFile file(imgPath);
	if (file.exists()) {
		_imgLabel->setImage(imgPath);
		_imgLabel->setHidden(false);
	}

}

void AddInstrumentDialog::accept() {
	QString name = _nameEdit->text();
	QString pinyin = _pinyinEdit->text().toUpper();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}
	if (pinyin.isEmpty()) {
		_pinyinEdit->setFocus();
		return;
	}

	InstrumentDao dao;
	InstrumentType it;
	it.typeName = name;

	it.isVip = _checkVIPBox->checkState();
	it.pinyin = pinyin;

	_waiter->start();
	if (_isModify)
	{
		it.typeId = _instrumentId.toInt();
		result_t res = dao.updateInstrumentType(it);
		if (res.isOk()) {
			if (!_imgFilePath.isEmpty())
			{
				uploadImg();
			}
			else
			{
				return QDialog::accept();
			}
		}
		else
			XNotifier::warn(QString("修改器械失败: ").append(res.msg()));
	}
	else
	{
		int typeId;
		result_t res = dao.addInstrumentType(it, &typeId);
		if (res.isOk())
		{
			if (!_imgFilePath.isEmpty())
			{
				_instrumentId = QString::number(typeId);
				uploadImg();
			}
			else
			{
				return QDialog::accept();
			}
		}
		else
			XNotifier::warn(QString("添加器械失败: ").append(res.msg()));
		
	}
	
}

void AddInstrumentDialog::loadImg() {
	QFileDialog *fileDialog = new QFileDialog(this);
	fileDialog->setWindowTitle(tr("打开图片"));
	fileDialog->setDirectory(".");
	fileDialog->setNameFilter(tr("Images(*.png *.jpg *.jpeg)"));
	fileDialog->setFileMode(QFileDialog::ExistingFiles);
	fileDialog->setViewMode(QFileDialog::Detail);

	QStringList fileNames;
	if (fileDialog->exec())
		fileNames = fileDialog->selectedFiles();

	if (fileNames.size() == 0 || fileNames.size() > 1) return;

	_imgFilePath = fileNames.at(0);
	_imgLabel->setImage(_imgFilePath);
	_imgLabel->setHidden(false);
}

void AddInstrumentDialog::uploadImg() {
	QString newFileName = QString("./photo/instrument/%1.jpg").arg(_instrumentId);
	if (!copyFileToPath(_imgFilePath, newFileName, true)) {
		XNotifier::warn(QString("器械信息添加成功，拷贝本地器械图片失败!"));
		return;
	}
	else
	{
		FtpManager::getInstance()->put(newFileName, newFileName);
	}
}

void AddInstrumentDialog::imgError(QNetworkReply::NetworkError error)
{
	qDebug() << error;
	XNotifier::warn(QString("上传器械图片失败: ").append(error));

	return QDialog::accept();
}

void AddInstrumentDialog::imgUploaded()
{
	QString md5 = getFileMd5(_imgFilePath);
	if (!md5.isEmpty())
	{
		InstrumentDao dao;

		result_t resp = dao.setInstrumentPhoto(_instrumentId.toInt(), md5);
		if (!resp.isOk())
		{
			XNotifier::warn(QString("上传器械图片失败: ").append(resp.msg()));
		}
	}

	return QDialog::accept();

}

bool AddInstrumentDialog::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
{
	toDir.replace("\\", "/");

	if (sourceDir == toDir) {
		return true;
	}

	if (!QFile::exists(sourceDir)) {
		return false;
	}

	QDir *createfile = new QDir;
	bool exist = createfile->exists(toDir);
	if (exist) {
		if (coverFileIfExist) {
			createfile->remove(toDir);
		}
	}

	return QFile::copy(sourceDir, toDir);
}

const QString AddInstrumentDialog::getFileMd5(const QString &filePath)
{
	QFile theFile(filePath);
	if (theFile.exists())
	{
		theFile.open(QIODevice::ReadOnly);
		QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
		theFile.close();
		return QString(ba.toHex().constData());
	}
	else
	{
		return QString("");
	}
}