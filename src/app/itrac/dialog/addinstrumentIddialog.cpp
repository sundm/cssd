#include "addinstrumentIddialog.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "xnotifier.h"
#include "barcode.h"
#include "widget/controls/idedit.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include "rdao/dao/InstrumentDao.h"
#include "ftpmanager.h"
#include <xui/images.h>
#include <qhttpmultipart.h>
#include <thirdparty/qjson/src/parser.h>
#include <QNetworkreply>
#include <QtWidgets/QtWidgets>

AddInstrumentIdDialog::AddInstrumentIdDialog(QWidget *parent)
	: QDialog(parent)
	, _aliasEdit(new Ui::FlatEdit)
	, _idEdit(new Ui::FlatEdit)
	, _insEdit(new InstrumentEdit)
	, _imgLabel(new XPicture(this))
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("添加新器械");

	_isModify = false;
	_imgFilePath.clear();

	_idEdit->setInputValidator(Ui::InputValitor::LetterAndNumber);

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &AddInstrumentIdDialog::accept);

	_imgLabel->setFixedHeight(256);
	_imgLabel->setBgColor(QColor(245, 246, 247));
	_imgLabel->setHidden(true);

	QPushButton *loadButton = new QPushButton("加载图片");
	loadButton->setIcon(QIcon(":/res/plus-24.png"));
	connect(loadButton, SIGNAL(clicked()), this, SLOT(loadImg()));

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("所属基础器械"), 0, 0);
	mainLayout->addWidget(new QLabel("器械UDI"), 1, 0);
	mainLayout->addWidget(new QLabel("器械别名"), 2, 0);

	mainLayout->addWidget(_insEdit, 0, 1);
	mainLayout->addWidget(_idEdit, 1, 1);
	mainLayout->addWidget(_aliasEdit, 2, 1);
	
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);
	
	mainLayout->addWidget(loadButton, 4, 0, 1, 2, Qt::AlignLeft);
	mainLayout->addWidget(_imgLabel, 5, 0, 1, 2);

	mainLayout->addWidget(submitButton, 6, 0, 1, 2, Qt::AlignHCenter);

	resize(600, 400);

	connect(_insEdit, SIGNAL(changed(int)), this, SLOT(onInstrumentTypeChange(int)));

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	connect(FtpManager::getInstance(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(imgError(QNetworkReply::NetworkError)));
	connect(FtpManager::getInstance(), SIGNAL(uploadFinished()), this, SLOT(imgUploaded()));

	//connect(_insEdit, SIGNAL(changed(int)), this, SLOT(onDeptChanged(int)));
	_insEdit->load();
}

void AddInstrumentIdDialog::setInfo(const QString &id)
{
	setWindowTitle("修改器械");
	_imgFilePath.clear();
	_isModify = true;
	_instrumentId = id;

	InstrumentDao dao;
	Instrument it;
	InstrumentType ity;
	result_t resp = dao.getInstrument(_instrumentId, &it);
	if (resp.isOk())
	{
		resp = dao.getInstrumentType(it.typeId, &ity);

		if (resp.isOk())
		{
			_aliasEdit->setText(it.alias);
			_idEdit->setText(_instrumentId);
			_insEdit->setReadOnly(_isModify);
			_idEdit->setReadOnly(_isModify);
			//_nameEdit->setReadOnly(_isModify);
			_insEdit->setCurrentIdPicked(ity.typeId, ity.typeName);

			QString imgPath = QString("./photo/instrument/%1.jpg").arg(_instrumentId);
			QFile file(imgPath);
			if (file.exists()) {
				_imgLabel->setImage(imgPath);
				_imgLabel->setHidden(false);
			}
			else
			{
				QString imgPath = QString("./photo/instrument/%1.jpg").arg(it.typeId);
				QFile file(imgPath);
				if (file.exists()) {
					_imgLabel->setImage(imgPath);
					_imgLabel->setHidden(false);
				}
			}
		}
		else
		{
			XNotifier::warn(QString("查询器械类型失败: ").append(resp.msg()));
		}
	}
	else
	{
		XNotifier::warn(QString("查询器械失败: ").append(resp.msg()));
	}
	
}

void AddInstrumentIdDialog::onInstrumentTypeChange(int typeId)
{
	QString imgPath = QString("./photo/instrument/%1.jpg").arg(typeId);
	QFile file(imgPath);
	if (file.exists()) {
		_imgLabel->setImage(imgPath);
		_imgLabel->setHidden(false);
	}
	else
	{
		_imgLabel->setImage("");
		_imgLabel->setHidden(true);
		resize(600, 400);
	}
}

void AddInstrumentIdDialog::loadImg() {
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

void AddInstrumentIdDialog::accept() {
	QString alias = _aliasEdit->text();
	_instrumentId = _idEdit->text().toUpper();
	int typeId = _insEdit->currentId();

	if (_instrumentId.isEmpty()) {
		_idEdit->setFocus();
		return;
	}
	if (typeId <= 0)
	{
		_insEdit->setFocus();
		return;
	}

	Instrument it;
	it.alias = alias;
	it.udi = _instrumentId;
	it.typeId = typeId;

	_waiter->start();
	if (_isModify)
	{
		InstrumentDao dao;
		result_t resp = dao.updateInstrument(it);
		_waiter->stop();
		if (resp.isOk())
		{
			emit reload();

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
		{
			XNotifier::warn(QString("修改器械失败: ").append(resp.msg()));
		}

	}
	else
	{
		InstrumentDao dao;
		result_t resp = dao.addInstrument(it);
		_waiter->stop();
		if (resp.isOk())
		{
			if (!_imgFilePath.isEmpty())
			{
				uploadImg();
			}

			resetView();

			emit reload();
		}
		else
		{
			XNotifier::warn(QString("添加器械失败: ").append(resp.msg()));
		}
	}
	
}

void AddInstrumentIdDialog::resetView()
{
	_idEdit->clear();
	_aliasEdit->clear();
}

void AddInstrumentIdDialog::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	
	if (tc.type() == TranspondCode::Instrument)
	{
		_idEdit->setText(code);
	}
	
}

void AddInstrumentIdDialog::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}

void AddInstrumentIdDialog::uploadImg() {
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

void AddInstrumentIdDialog::imgError(QNetworkReply::NetworkError error)
{
	qDebug() << error;
	XNotifier::warn(QString("上传器械图片失败: ").append(error));

	return QDialog::accept();
}

void AddInstrumentIdDialog::imgUploaded()
{
	QString md5 = getFileMd5(_imgFilePath);
	if (!md5.isEmpty())
	{
		InstrumentDao dao;

		result_t resp = dao.setInstrumentPhoto(_instrumentId, md5);
		if (!resp.isOk())
		{
			XNotifier::warn(QString("上传器械图片失败: ").append(resp.msg()));
		}
	}

	return QDialog::accept();
	
}

bool AddInstrumentIdDialog::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
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

const QString AddInstrumentIdDialog::getFileMd5(const QString &filePath)
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