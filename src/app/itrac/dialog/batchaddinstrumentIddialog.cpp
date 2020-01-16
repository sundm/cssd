#include "batchaddinstrumentIddialog.h"
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
#include "ui/views.h"
#include <qhttpmultipart.h>
#include <thirdparty/qjson/src/parser.h>
#include <QNetworkreply>
#include <QtWidgets/QtWidgets>

BatchAddInstrumentIdDialog::BatchAddInstrumentIdDialog(QWidget *parent)
	: QDialog(parent)
	, _insEdit(new InstrumentEdit)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 1, _view))
	, _imgLabel(new XPicture(this))
	, _waiter(new WaitingSpinner(this))
{
	_model->setHeaderData(0, Qt::Horizontal, "器械UDI");

	_view->setModel(_model);
	_view->setMinimumHeight(500);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);

	setWindowTitle("批量添加新器械");

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &BatchAddInstrumentIdDialog::accept);

	_imgLabel->setFixedHeight(256);
	_imgLabel->setBgColor(QColor(245, 246, 247));
	_imgLabel->setHidden(true);

	QPushButton *loadButton = new QPushButton("加载图片");
	loadButton->setIcon(QIcon(":/res/plus-24.png"));
	connect(loadButton, SIGNAL(clicked()), this, SLOT(loadImg()));

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("所属基础器械"), 0, 0);
	mainLayout->addWidget(new QLabel("器械列表"), 1, 0);
	
	mainLayout->addWidget(_insEdit, 0, 1);
	mainLayout->addWidget(_view, 1, 1);
	
	
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 2, 0, 1, 2);
	
	mainLayout->addWidget(loadButton, 3, 0, 1, 2, Qt::AlignLeft);
	mainLayout->addWidget(_imgLabel, 4, 0, 1, 2);

	mainLayout->addWidget(submitButton, 5, 0, 1, 2, Qt::AlignHCenter);

	resize(parent ? parent->width() / 3 : 360, sizeHint().height());

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	connect(_insEdit, SIGNAL(changed(int)), this, SLOT(onInsTypeChange(int)));
	_insEdit->load();
}

void BatchAddInstrumentIdDialog::onInsTypeChange(int type)
{
	_model->removeRows(0, _model->rowCount());
	_imgLabel->setHidden(true);
}

void BatchAddInstrumentIdDialog::loadImg() {
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

void BatchAddInstrumentIdDialog::accept() {
	int typeId = _insEdit->currentId();
	if (typeId <= 0)
	{
		_insEdit->setFocus();
		return;
	}

	_waiter->start();
	QMap<int, int> rowMap;
	bool success = true;
	for (size_t i = 0; i < _model->rowCount(); i++)
	{
		Instrument it;
		it.udi = _model->data(_model->index(i, 0)).toString();
		it.typeId = typeId;

		InstrumentDao dao;
		result_t resp = dao.addInstrument(it);

		if (resp.isOk())
		{
			if (!_imgFilePath.isEmpty())
			{
				uploadImg(it.udi);
			}

			rowMap.insert(i, 0);
		}
		else
		{
			success = false;
		}
	}

	_waiter->stop();

	if (!success)
	{
		int rowToDel;
		QMapIterator<int, int> rowMapIterator(rowMap);
		rowMapIterator.toBack();
		while (rowMapIterator.hasPrevious())
		{
			rowMapIterator.previous();
			rowToDel = rowMapIterator.key();
			_model->removeRow(rowToDel);
			_scannedList.removeAt(rowToDel);
		}

		XNotifier::warn(QString("以下器械UDI添加失败!"));
	}
	
	
	emit reload();
}

void BatchAddInstrumentIdDialog::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	
	if (tc.type() == TranspondCode::Instrument)
	{
		if (_scannedList.contains(code))
		{
			return;
		}

		int count = _model->rowCount();
		_model->insertRows(count, 1);
		_model->setData(_model->index(count, 0), code);
		_scannedList.append(code);
	}
	
}

void BatchAddInstrumentIdDialog::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}

void BatchAddInstrumentIdDialog::uploadImg(const QString& instrument_id) {
	QString newFileName = QString("./photo/instrument/%1.jpg").arg(instrument_id);
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
			QString newFileName = QString("./photo/instrument/%1.jpg").arg(instrument_id);
			if (!copyFileToPath(_imgFilePath, newFileName, true)) {
				XNotifier::warn(QString("器械信息添加成功，拷贝本地器械图片失败!"));
				return;
			}

		}
	}
	*/
}

bool BatchAddInstrumentIdDialog::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
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