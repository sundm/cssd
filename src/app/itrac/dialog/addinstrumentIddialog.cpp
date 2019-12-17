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

	resize(parent ? parent->width() / 3 : 360, sizeHint().height());

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	//connect(_insEdit, SIGNAL(changed(int)), this, SLOT(onDeptChanged(int)));
	_insEdit->load();
}

void AddInstrumentIdDialog::setInfo(const QString &id)
{
	setWindowTitle("修改器械");
	_isModify = true;
	_instrumentId = id;

	InstrumentDao dao;
	Instrument it;
	InstrumentType ity;
	result_t resp = dao.getInstrument(_instrumentId, &it);
	dao.getInstrumentType(it.typeId, &ity);

	if (resp.isOk())
	{
		_aliasEdit->setText(it.alias);
		_idEdit->setText(_instrumentId);
		_idEdit->setReadOnly(_isModify);
		//_nameEdit->setReadOnly(_isModify);
		_insEdit->setCurrentIdPicked(ity.typeId, ity.typeName);

		QString imgPath = QString("./photo/instrument/%1.png").arg(_instrumentId);
		QFile file(imgPath);
		if (file.exists()) {
			_imgLabel->setImage(imgPath);
			_imgLabel->setHidden(false);
		}
	}
	else
	{
		XNotifier::warn(QString("查询器械失败: ").append(resp.msg()));
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
	QString udi = _idEdit->text().toUpper();
	int typeId = _insEdit->currentId();

	if (udi.isEmpty()) {
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
	it.udi = udi;
	it.typeId = typeId;

	_waiter->start();
	if (_isModify)
	{
		InstrumentDao dao;
		result_t resp = dao.updateInstrument(it);
		_waiter->stop();
		if (resp.isOk())
		{
			if (!_imgFilePath.isEmpty())
			{
				uploadImg(udi);
			}

			return QDialog::accept();
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
				uploadImg(udi);
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
	/*QString name = _nameEdit->text();
	if (name.isEmpty()) return;
	
	QStringList l = name.split("#");
	if (l.count() == 2)
	{
		bool isOK = false;
		int num = l.at(1).toInt(&isOK);
		if (isOK)
		{
			_nameEdit->setText(QString("%1#%2").arg(l.at(0)).arg(QString::number(num + 1)));
		}
	}

	_nameEdit->setFocus();*/
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

void AddInstrumentIdDialog::uploadImg(const QString& instrument_id) {
	QString newFileName = QString("./photo/instrument/%1.png").arg(instrument_id);
	if (!copyFileToPath(_imgFilePath, newFileName, true)) {
		XNotifier::warn(QString("器械信息添加成功，拷贝本地器械图片失败!"));
		return;
	}

	/*
	_multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	QHttpPart imagePart;
	imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
	QString head = QString("form-data; name=\"file\"; filename=\"%1.png\"").arg(instrument_id);
	imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(head));
	_imgFile = new QFile(_imgFilePath);
	_imgFile->open(QIODevice::ReadOnly);
	imagePart.setBodyDevice(_imgFile);
	_imgFile->setParent(_multiPart);
	_multiPart->append(imagePart);

	const QByteArray resp = post(url(PATH_INSTRUMENT_UPLOAD_IMG), _multiPart);

	QJson::Parser parser;
	bool ok;
	QVariantMap vmap = parser.parse(resp, &ok).toMap();
	if (!ok) {
		XNotifier::warn(QString("上传器械图片失败"));
		return;
	}
	else {
		QString code = vmap.value("code").toString();
		if ("9000" != code) {
			XNotifier::warn(QString("上传器械图片失败:").append(code));
			return;
		}
		else {
			QString newFileName = QString("./photo/instrument/%1.png").arg(instrument_id);
			if (!copyFileToPath(_imgFilePath, newFileName, true)) {
				XNotifier::warn(QString("器械信息添加成功，拷贝本地器械图片失败!"));
				return;
			}

		}
	}
	*/
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