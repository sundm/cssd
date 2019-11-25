#include "addpkgcodedialog.h"
#include "xnotifier.h"
#include "core/constants.h"
#include "barcode.h"
#include "core/user.h"
#include "core/net/url.h"
#include "ui/groups.h"
#include "ui/inputfields.h"
#include "ui/buttons.h"
#include "widget/controls/combos.h"
#include "widget/controls/idedit.h"
#include "ui/views.h"
#include "model/spinboxdelegate.h"
#include "ui/composite/waitingspinner.h"
#include <thirdparty/qjson/src/parser.h>
#include <xui/images.h>
#include <xui/imageviewer.h>
#include <qhttpmultipart.h>
#include <qprocess.h>
#include <QNetworkreply>
#include <QtWidgets/QtWidgets>
#include <QDateTime>
#include <QTimer>
#include <QSpinBox>

AddpkgcodeDialog::AddpkgcodeDialog(QWidget *parent)
	: QDialog(parent)
	, _pkgNameEdit(new Ui::FlatEdit)
	, _pkgCodeEdit(new Ui::FlatEdit)
	, _pkgEdit(new PackageEdit)
	, _insRfidEdit(new Ui::FlatEdit)
	, _imgLabel(new XPicture(this))
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 2, _view))
	, _unview(new TableView(this))
	, _unmodel(new QStandardItemModel(0, 2, _unview))
	, _waiter(new WaitingSpinner(this))
{
	_isModify = false;
	//_pkgNameEdit->setText(pkg_name);
	//_pkgNameEdit->setReadOnly(true);
	FormGroup * pkgGroup = new FormGroup(this);
	pkgGroup->addRow("包名称:", _pkgNameEdit);
	pkgGroup->addRow("包UID:", _pkgCodeEdit);
	pkgGroup->addRow("包类型:", _pkgEdit);
	//_package_type_id = pkg_id;

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(_insRfidEdit);

	QToolButton *addButton = new QToolButton;
	addButton->setIcon(QIcon(":/res/plus-24.png"));
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	QToolButton *minusButton = new QToolButton;
	minusButton->setIcon(QIcon(":/res/delete-24.png"));
	minusButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	FormGroup * viewGroup = new FormGroup(this);
	viewGroup->addRow("器械列表",_view);
	viewGroup->addRow("异常器械", _unview);

	setWindowTitle("包内器械信息");
	_commitButton = new Ui::PrimaryButton("确定", Ui::BtnSize::Small);
	connect(_commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	_resetButton = new Ui::PrimaryButton("重置", Ui::BtnSize::Small);
	connect(_resetButton, SIGNAL(clicked()), this, SLOT(reset()));

	_loadImgButton = new Ui::PrimaryButton("加载图片", Ui::BtnSize::Small);
	connect(_loadImgButton, SIGNAL(clicked()), this, SLOT(loadImg()));

	_imgLabel->setFixedHeight(256);
	_imgLabel->setBgColor(QColor(245, 246, 247));
	_imgLabel->setHidden(true);

	QVBoxLayout *avlayout = new QVBoxLayout();
	avlayout->addWidget(pkgGroup);
	avlayout->addWidget(viewGroup);

	QVBoxLayout *bvlayout = new QVBoxLayout();
	bvlayout->addWidget(_imgLabel);
	bvlayout->addWidget(_loadImgButton);

	QHBoxLayout *cLayout = new QHBoxLayout;
	cLayout->addLayout(avlayout);
	cLayout->addLayout(bvlayout);
	cLayout->setStretch(0, 3);
	cLayout->setStretch(1, 2);


	QHBoxLayout *bLayout = new QHBoxLayout;
	bLayout->addWidget(_resetButton);
	bLayout->addWidget(_commitButton);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addLayout(cLayout);
	layout->addLayout(bLayout);

	initInstrumentView();

	resize(1000, 900);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	QTimer::singleShot(1000, this, &AddpkgcodeDialog::initData);
	QTimer::singleShot(500, [this] {_pkgEdit->loadForDepartment(0);});
}

void AddpkgcodeDialog::initInstrumentView() {
	_model->setHeaderData(0, Qt::Horizontal, "器械类型");
	_model->setHeaderData(1, Qt::Horizontal, "器械UID");
	_view->setModel(_model);
	_view->setMinimumHeight(500);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 50);

	_unmodel->setHeaderData(0, Qt::Horizontal, "器械类型");
	_unmodel->setHeaderData(1, Qt::Horizontal, "器械UID");
	_unview->setModel(_unmodel);
	_unview->setMinimumHeight(200);

	QHeaderView *unheader = _unview->horizontalHeader();
	unheader->setStretchLastSection(true);
	unheader->resizeSection(0, 150);
	unheader->resizeSection(1, 50);
}

void AddpkgcodeDialog::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
}

void AddpkgcodeDialog::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}

void AddpkgcodeDialog::loadImg() {
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

void AddpkgcodeDialog::uploadImg(int instrument_id) {
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

bool AddpkgcodeDialog::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
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

void AddpkgcodeDialog::initData() {
	if (_package_id.isEmpty()) return;

	QByteArray data("{\"pkg_type_id\":");
	data.append(_package_id).append('}');
	//post(url(PATH_CARD_SEARCH), QByteArray().append(data), [=](QNetworkReply *reply) {
	//	JsonHttpResponse resp(reply);
	//	if (!resp.success()) {
	//		XNotifier::warn(QString("无法获取包内器械列表信息: ").append(resp.errorString()));
	//		return;
	//	}

	//	QList<QVariant> orders = resp.getAsList("card_info");

	//	for (auto &order : orders) {
	//		QVariantMap map = order.toMap();
	//		int code = map["card_id"].toInt();

	//		QList<QStandardItem *> items;
	//		QStandardItem *insItem = new QStandardItem(QString::number(code));
	//		insItem->setData(code);
	//		items << insItem;
	//		_model->appendRow(items);
	//	}
	//});
}

//void AddpkgcodeDialog::addEntry() {
//	Barcode bc(_pkgCodeEdit->text());
//	if (bc.type() == Barcode::PkgCode) {
//		_code = bc.intValue();
//		int existRow = findRow(_code);
//		if (-1 == existRow) {
//			QVariantList codes;
//			
//			QVariantMap code_map;
//			code_map.insert("card_id", _code);
//			code_map.insert("card_name", "");//todo
//			codes << code_map;
//			
//
//			QVariantMap data;
//			data.insert("pkg_type_id", _package_type_id);
//			data.insert("cards", codes);
//
//			post(url(PATH_CARD_ADD), data, [this](QNetworkReply *reply) {
//				JsonHttpResponse resp(reply);
//				if (!resp.success()) {
//					XNotifier::warn(QString("添加包内器械表失败: ").append(resp.errorString()));
//					return;
//				}
//
//				QList<QStandardItem *> items;
//				QStandardItem *insItem = new QStandardItem(QString::number(_code));
//				insItem->setData(_code);
//				items << insItem;
//				_model->appendRow(items);
//			});
//
//
//			
//		}
//		else {
//			XNotifier::warn(QString("输入的编号已存在!"));
//			return;
//		}
//		_pkgCodeEdit->clear();
//	}
//}
//
//void AddpkgcodeDialog::removeEntry() {
//	QItemSelectionModel *selModel = _view->selectionModel();
//	QModelIndexList indexes = selModel->selectedRows();
//	int countRow = indexes.count();
//	for (int i = countRow; i > 0; i--)
//		_model->removeRow(indexes.at(i - 1).row());
//}

void AddpkgcodeDialog::accept() {
	

	if (_isModify)
	{
		//todo
	}
	else {
		//todo
	}
	return QDialog::accept();
}

void AddpkgcodeDialog::reset()
{
	//todo
}

void AddpkgcodeDialog::setPackageId(const QString &pkgId)
{
	_package_id = pkgId;
	_isModify = true;

	QTimer::singleShot(1000, this, &AddpkgcodeDialog::initData);
}

int AddpkgcodeDialog::findRow(int code) {
	QModelIndexList matches = _model->match(_model->index(0, 0), Qt::UserRole + 1, code, -1);
	for (const QModelIndex &index : matches) {
		return index.row();
	}
	return -1;
}