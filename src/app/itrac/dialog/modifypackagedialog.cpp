#include "modifypackagedialog.h"
#include "xnotifier.h"
#include "core/constants.h"
#include "core/user.h"
#include "core/net/url.h"
#include "ui/groups.h"
#include "ui/inputfields.h"
#include "ui/buttons.h"
#include "widget/controls/combos.h"
#include "widget/controls/idedit.h"
#include "ui/views.h"
#include "model/itemdelegate.h"
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

ModifyPackageDialog::ModifyPackageDialog(QWidget *parent, const PackageInfo info, const QList<QVariant> orders)
	: QDialog(parent)
	, _pkgNameEdit(new Ui::FlatEdit)
	, _pkgRFIDCodeEdit(new Ui::FlatEdit)
	, _pkgtypeBox(new QComboBox)
	, _picktypeBox(new QComboBox)
	, _stertypeBox(new QComboBox)
	, _deptEdit(new DeptEdit)
	, _insEdit(new InstrumentEdit)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 2, _view))
	, _imgLabel(new XPicture(this))
{
	_info = info;
	_orders = orders;

	_package_type_id = info.package_type_id.toInt();
	_pkgNameEdit->setText(_info.package_name);
	_pkgNameEdit->setReadOnly(true);
	_pkgRFIDCodeEdit->setText(_info.package_pinyin);
	_pkgRFIDCodeEdit->setReadOnly(true);

	FormGroup * pkgGroup = new FormGroup(this);
	pkgGroup->addRow("包名 (*)", _pkgNameEdit);
	pkgGroup->addRow("包ID (*)", _pkgRFIDCodeEdit);
	pkgGroup->addRow("包类型 (*)", _pkgtypeBox);
	pkgGroup->addRow("打包类型 (*)", _picktypeBox);
	pkgGroup->addRow("高低温灭菌 (*)", _stertypeBox);
	pkgGroup->addRow("所属科室 (*)", _deptEdit);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(_insEdit);

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
	QWidget *w = new QWidget(viewGroup);
	w->setLayout(hLayout);
	viewGroup->addRow("添加器械 (*)", w);
	viewGroup->addRow("器械列表",_view);

	setWindowTitle("修改包信息");
	_commitButton = new Ui::PrimaryButton("修改", Ui::BtnSize::Small);
	connect(_commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	_imgLabel->setFixedHeight(256);
	_imgLabel->setBgColor(QColor(245, 246, 247));
	_imgLabel->setHidden(true);
	//connect(_imgLabel, SIGNAL(clicked()), this, SLOT(imgClicked()));

	_loadImgButton = new Ui::PrimaryButton("加载图片", Ui::BtnSize::Small);
	connect(_loadImgButton, SIGNAL(clicked()), this, SLOT(loadImg()));

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

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addLayout(cLayout);
	layout->addWidget(_commitButton);

	initInstrumentView();

	resize(900, sizeHint().height());

	QTimer::singleShot(0, this, &ModifyPackageDialog::initData);
}

void ModifyPackageDialog::initInstrumentView() {
	_model->setHeaderData(0, Qt::Horizontal, "器械名称");
	_model->setHeaderData(1, Qt::Horizontal, "数量");
	_view->setModel(_model);
	_view->setMinimumHeight(500);

	_view->setItemDelegate(new SpinBoxDelegate(
		1, Constant::maxPackageCount, Constant::minPackageCount, _view));

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 50);

	for (int i = 0; i != _orders.count(); ++i) {
		QVariantMap map = _orders[i].toMap();
		QList<QStandardItem *> items;
		QStandardItem *insItem = new QStandardItem(map["instrument_name"].toString());
		insItem->setData(map["instrument_id"].toInt());
		items << insItem << new QStandardItem(map["instrument_number"].toString());
		_model->appendRow(items);
	}
}

void ModifyPackageDialog::initData() {
	_pkgtypeBox->addItem(QString("手术包"), 0);
	_pkgtypeBox->addItem(QString("临床包"), 1);
	_pkgtypeBox->addItem(QString("外来包"), 2);
	_pkgtypeBox->addItem(QString("敷料包"), 3);
	_pkgtypeBox->addItem(QString("通用包"), 4);

	_pkgtypeBox->setCurrentIndex(_info.package_category.toInt());

	_stertypeBox->addItem(QString("通用"), 0);
	_stertypeBox->addItem(QString("高温"), 1);
	_stertypeBox->addItem(QString("低温"), 2);
	_stertypeBox->setCurrentIndex(_info.steType);

	_picktypeBox->clear();

	QString data = QString("{}");

	post(url(PATH_PACKTYPE_SEARCH), QByteArray().append(data), [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
		}

		_picktypeBox->addItem(QString("不限"), 0);

		QList<QVariant> devices = resp.getAsList("pack_types");
		for (auto &device : devices) {
			QVariantMap map = device.toMap();
			_picktypeBox->addItem(map["pack_type_name"].toString(), map["pack_type_id"].toInt());
		}

		_picktypeBox->setCurrentText(_info.pack_type);
	});

	_deptEdit->load(DeptEdit::ALL);
	_deptEdit->setCurrentIdPicked(_info.dtp_id.toInt(), _info.dtp_name);

	_insEdit->load();

	QString imgPath = QString("./photo/package/%1.png").arg(_info.package_type_id);
	_imgLabel->setImage(imgPath);
	_imgLabel->setHidden(false);
}

void ModifyPackageDialog::addEntry() {
	int insId = _insEdit->currentId();

	if (insId == 0) return;

	int existRow = findRow(insId);
	if (-1 == existRow) {
		QList<QStandardItem *> items;
		QStandardItem *insItem = new QStandardItem(_insEdit->currentName());
		insItem->setData(_insEdit->currentId());
		items << insItem << new QStandardItem("1");
		_model->appendRow(items);
	}
	else {
		QStandardItem *countItem = _model->item(existRow, 1);
		int count = countItem->text().toInt();
		if (count < Constant::maxPackageCount) {
			countItem->setText(QString::number(count + 1));
			_view->selectRow(existRow);
		}
	}
	_insEdit->reset();
}

void ModifyPackageDialog::removeEntry() {
	QItemSelectionModel *selModel = _view->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_model->removeRow(indexes.at(i - 1).row());
}

void ModifyPackageDialog::accept() {
	QString package_type_name = _pkgNameEdit->text();
	if (package_type_name.isEmpty()) {
		_pkgNameEdit->setFocus();
		return;
	}
		

	QString pinyin_code = _pkgRFIDCodeEdit->text();
	if (pinyin_code.isEmpty()) {
		_pkgRFIDCodeEdit->setFocus();
		return;
	}
		

	QString package_category = _pkgtypeBox->currentData().toString();

	int pack_type_id = _picktypeBox->currentData().toInt();
	if (0 == pack_type_id) {
		_picktypeBox->setFocus();
		return;
	}

	int sterilize_type = _stertypeBox->currentData().toInt();

	int department_id = _deptEdit->currentId();
	if (_deptEdit->currentName().isEmpty()) {
		_deptEdit->setFocus();
		return;
	}

	QVariantList instruments;
	for (int i = 0; i != _model->rowCount(); i++) {
		QVariantMap ext_info_map;
		ext_info_map.insert("instrument_id", _model->item(i, 0)->data().toInt());
		ext_info_map.insert("instrument_number", _model->item(i, 1)->text().toInt());
		instruments << ext_info_map;
	}

	QVariantMap data;
	data.insert("package_type_id", _package_type_id);
	data.insert("package_type_name", package_type_name);
	data.insert("package_category", package_category);
	data.insert("pinyin_code", pinyin_code);
	data.insert("pack_type_id", pack_type_id);
	data.insert("department_id", department_id);
	data.insert("sterilize_type", sterilize_type);
	data.insert("instruments", instruments);

	post(url(PATH_PKGTPYE_MODIFY), data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("更新包记录失败: ").append(resp.errorString()));
			return;
		}
		
		//upload img
		if (!_imgFilePath.isEmpty())
		{
			if (0 != _package_type_id) uploadImg();
			return QDialog::accept();
		}
		else {
			return QDialog::accept();
		}
	});
}
/*
void AddPackageDialog::setInfo(const QString& pkg_type_id) {
	QVariantMap data;
	data.insert("package_type_id", pkg_type_id);

	post(url(PATH_PKGTPYE_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("获取包信息失败: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> pkgs = resp.getAsList("package_types");
		if (pkgs.count() != 1)
		{
			XNotifier::warn(QString("获取包信息失败。"));
			return;
		}



		for (int i = 0; i != pkgs.count(); ++i) {
			QVariantMap map = pkgs[i].toMap();
			
			_package_type_id = map["package_type_id"].toInt();

			QString package_name = map["package_name"].toString();
			_pkgNameEdit->setText(package_name);
			_pkgNameEdit->setReadOnly(true);
			
			QString pinyin_code = map["pinyin_code"].toString();
			_pkgPinYinCodeEdit->setText(pinyin_code);

			int package_category = map["package_category"].toInt();
			_pkgtypeBox->setCurrentIndex(_pkgtypeBox->findData(package_category));

			QString pack_type = map["pack_type"].toString();
			int data = _picktypeBox->findData(pack_type);
			int text = _picktypeBox->findText(pack_type);

			int sterilize_type = map["sterilize_type"].toInt();
			_stertypeBox->setCurrentIndex(_stertypeBox->findData(sterilize_type));
		}
	});
}
*/

void ModifyPackageDialog::uploadImg() {
	_multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	QHttpPart imagePart;
	imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
	QString head = QString("form-data; name=\"file\"; filename=\"%1.png\"").arg(_package_type_id);
	imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(head));
	_imgFile = new QFile(_imgFilePath);
	_imgFile->open(QIODevice::ReadOnly);
	imagePart.setBodyDevice(_imgFile);
	_imgFile->setParent(_multiPart);
	_multiPart->append(imagePart);

	const QByteArray resp = post(url(PATH_PKGTPYE_UPLOAD_IMG), _multiPart);

	QJson::Parser parser;
	bool ok;
	QVariantMap vmap = parser.parse(resp, &ok).toMap();
	if (!ok) {
		XNotifier::warn(QString("上传包图片失败"));
		return;
	}
	else {
		QString code = vmap.value("code").toString();
		if ("9000" != code) {
			XNotifier::warn(QString("上传包图片失败:").append(code));
			return;
		}
		else {
			QString newFileName = QString("./photo/package/%1.png").arg(_package_type_id);
			if (!copyFileToPath(_imgFilePath, newFileName, true)) {
				XNotifier::warn(QString("包信息添加成功，拷贝本地包图片失败!"));
				return;
			}

		}
	}
}

bool ModifyPackageDialog::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
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

void ModifyPackageDialog::loadImg() {
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

/*
void AddPackageDialog::imgClicked()
{
	ImageViewer *viewer = new ImageViewer(_imgLabel->fileName());
	viewer->showNormal();
}
*/

void ModifyPackageDialog::loadData() {

}

int ModifyPackageDialog::findRow(int insId) {
	QModelIndexList matches = _model->match(_model->index(0, 0), Qt::UserRole + 1, insId, -1);
	for (const QModelIndex &index : matches) {	
		return index.row();
	}
	return -1;
}
