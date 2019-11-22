#include "addinstrumentIddialog.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "xnotifier.h"
#include "widget/controls/idedit.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include <xui/images.h>
#include <qhttpmultipart.h>
#include <thirdparty/qjson/src/parser.h>
#include <QNetworkreply>
#include <QtWidgets/QtWidgets>

AddInstrumentIdDialog::AddInstrumentIdDialog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _idEdit(new Ui::FlatEdit)
	, _insEdit(new InstrumentEdit)
	, _imgLabel(new XPicture(this))
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("添加新器械");

	_isModify = false;

	_idEdit->setInputValidator(Ui::InputValitor::LetterOnly);

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
	mainLayout->addWidget(new QLabel("器械名"), 0, 0);
	mainLayout->addWidget(new QLabel("器械UID"), 1, 0);
	mainLayout->addWidget(new QLabel("所属基础器械"), 2, 0);
	
	mainLayout->addWidget(_nameEdit, 0, 1);
	mainLayout->addWidget(_idEdit, 1, 1);
	mainLayout->addWidget(_insEdit, 2, 1);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);
	
	mainLayout->addWidget(loadButton, 4, 0, 1, 2, Qt::AlignLeft);
	mainLayout->addWidget(_imgLabel, 5, 0, 1, 2);

	mainLayout->addWidget(submitButton, 6, 0, 1, 2, Qt::AlignHCenter);

	resize(parent ? parent->width() / 3 : 360, sizeHint().height());

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	_insEdit->load();
}

void AddInstrumentIdDialog::setInfo(const QString &id, const QString &name, const QString &basics)
{
	setWindowTitle("修改器械");
	_isModify = true;
	_instrumentId = id;

	_nameEdit->setText(name);
	_idEdit->setText(_instrumentId);
	_nameEdit->setReadOnly(_isModify);

	QString imgPath = QString("./photo/instrument/%1.png").arg(_instrumentId);
	QFile file(imgPath);
	if (file.exists()) {
		_imgLabel->setImage(imgPath);
		_imgLabel->setHidden(false);
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
	QString name = _nameEdit->text();
	QString pinyin = _idEdit->text().toUpper();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}
	if (pinyin.isEmpty()) {
		_idEdit->setFocus();
		return;
	}

	QVariantMap vmap;
	vmap.insert("instrument_name", name);
	vmap.insert("pinyin_code", pinyin);

	_waiter->start();
	if (_isModify)
	{
		vmap.insert("instrument_id", _instrumentId);
		post(url(PATH_INSTRUMENT_MODIFY), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("修改器械失败: ").append(resp.errorString()));
			}
			else {
				if (!_imgFilePath.isEmpty())
				{
					int instrument_id = _instrumentId.toInt();
					if (0 != instrument_id) uploadImg(instrument_id);
					return QDialog::accept();
				}
				else {
					return QDialog::accept();
				}
			}
		});
	}
	else
	{
		post(url(PATH_INSTRUMENT_ADD), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("添加器械失败: ").append(resp.errorString()));
			}
			else {
				if (!_imgFilePath.isEmpty())
				{
					int instrument_id = 0;
					instrument_id = resp.getAsInt("instrument_id");
					if (0 != instrument_id) uploadImg(instrument_id);
					return QDialog::accept();
				}
				else {
					return QDialog::accept();
				}
			}
		});
	}
	
}

void AddInstrumentIdDialog::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
}

void AddInstrumentIdDialog::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}

void AddInstrumentIdDialog::uploadImg(int instrument_id) {
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