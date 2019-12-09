#include "addpackagedialog.h"
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

AddPackageDialog::AddPackageDialog(QWidget *parent)
	: QDialog(parent)
	, _pkgNameEdit(new Ui::FlatEdit)
	, _pkgPYCodeEdit(new Ui::FlatEdit)
	, _pkgtypeBox(new QComboBox)
	, _picktypeBox(new QComboBox)
	, _stertypeBox(new QComboBox)
	, _importBox(new QCheckBox)
	, _deptEdit(new DeptEdit)
	, _insEdit(new InstrumentEdit)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 2, _view))
{
	_isModfy = false;

	FormGroup * pkgGroup = new FormGroup(this);
	pkgGroup->addRow("包名 (*)", _pkgNameEdit);
	pkgGroup->addRow("拼音检索码 (*)", _pkgPYCodeEdit);
	pkgGroup->addRow("包类型 (*)", _pkgtypeBox);
	pkgGroup->addRow("是否用于植入性手术 (*)", _importBox);
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
	
	QVBoxLayout *vlayout = new QVBoxLayout;
	vlayout->addLayout(hLayout);
	vlayout->addWidget(_view);

	setWindowTitle("添加包信息");
	_commitButton = new Ui::PrimaryButton("确定", Ui::BtnSize::Small);
	connect(_commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(pkgGroup);
	layout->addLayout(vlayout);
	layout->addWidget(_commitButton);

	resize(600, 800);

	initInstrumentView();

	connect(_pkgtypeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(typeBoxChanaged(int)));
	QTimer::singleShot(0, this, &AddPackageDialog::initData);

}

void AddPackageDialog::typeBoxChanaged(int index)
{
	switch (index)
	{
	case Rt::PackageCategory::SurgicalPackage:
		_importBox->setEnabled(true);
		break;
	case Rt::PackageCategory::ClinicalPackage:
		_importBox->setChecked(false);
		_importBox->setEnabled(false);
		break;
	case Rt::PackageCategory::ExternalPackage:
		_importBox->setChecked(true);
		_importBox->setEnabled(false);
		break;
	case Rt::PackageCategory::DressingPackage:
		_importBox->setChecked(false);
		_importBox->setEnabled(false);
		break;
	case Rt::PackageCategory::UniversalPackage:
		_importBox->setChecked(false);
		_importBox->setEnabled(false);
		break;
	default:
		break;
	}
}

void AddPackageDialog::initData() {
	_pkgtypeBox->addItem(QString("手术包"), Rt::PackageCategory::SurgicalPackage);
	_pkgtypeBox->addItem(QString("临床包"), Rt::PackageCategory::ClinicalPackage);
	_pkgtypeBox->addItem(QString("外来包"), Rt::PackageCategory::ExternalPackage);
	_pkgtypeBox->addItem(QString("敷料包"), Rt::PackageCategory::DressingPackage);
	_pkgtypeBox->addItem(QString("通用包"), Rt::PackageCategory::UniversalPackage);

	_stertypeBox->addItem(QString("通用"), Rt::SterilizeMethod::BothTemperature);
	_stertypeBox->addItem(QString("低温"), Rt::SterilizeMethod::LowTemperature);
	_stertypeBox->addItem(QString("高温"), Rt::SterilizeMethod::HighTemperature);

	_picktypeBox->clear();

	PackageDao dao;
	QList<PackType> packTypes;
	result_t resp = dao.getPackTypeList(&packTypes);
	if (resp.isOk())
	{
		_picktypeBox->addItem(QString("不限"), 0);
		for (auto &pkt : packTypes) {
			_picktypeBox->addItem(pkt.name, pkt.id);
			}
	}
	else
	{

	}

	_deptEdit->load(DeptEdit::ALL);
	_insEdit->load();
}

void AddPackageDialog::accept() {
	QString package_type_name = _pkgNameEdit->text();
	if (package_type_name.isEmpty()) {
		_pkgNameEdit->setFocus();
		return;
	}
		
	QString pinyin_code = _pkgPYCodeEdit->text();
	if (pinyin_code.isEmpty()) {
		_pkgPYCodeEdit->setFocus();
		return;
	}

	Rt::PackageCategory package_category = (Rt::PackageCategory)_pkgtypeBox->currentData().toInt();

	//int pack_type_id = _picktypeBox->currentData().toInt();
	//if (0 == pack_type_id) {
	//	_picktypeBox->showPopup();
	//	return;
	//}

	PackType pt;
	pt.id = _picktypeBox->currentData().toInt();
	pt.name = _picktypeBox->currentText();

	Rt::SterilizeMethod sterType = (Rt::SterilizeMethod)_stertypeBox->currentData().toInt();

	Department dept;

	dept.id = _deptEdit->currentId();
	if (_deptEdit->currentName().isEmpty()) {
		_deptEdit->setFocus();
		return;
	}
	dept.name = _deptEdit->currentName();

	getOrders();
	
	PackageDao dao;
	PackageType pkt;
	pkt.name = package_type_name;
	pkt.pinyin = pinyin_code;
	pkt.packType = pt;
	pkt.sterMethod = sterType;
	pkt.category = package_category;
	pkt.dept = dept;
	pkt.detail = _orders;

	if (_isModfy)
	{
		pkt.typeId = _package_type_id;
		result_t resp = dao.addPackageType(pkt);
		if (resp.isOk())
		{
			return QDialog::accept();
		}
		else
		{
			XNotifier::warn(QString("更新包失败: ").append(resp.msg()));
			return;
		}
	}
	else
	{
		result_t resp = dao.addPackageType(pkt);
		if (resp.isOk())
		{
			return QDialog::accept();
		}
		else
		{
			XNotifier::warn(QString("添加包失败: ").append(resp.msg()));
			return;
		}
	}
	
}

void AddPackageDialog::setInfo(const QString& pkg_type_id) {
	_package_type_id = pkg_type_id.toInt();
	_isModfy = true;
	setWindowTitle("修改包信息");
	QTimer::singleShot(1000, this, &AddPackageDialog::initPackageInfo);
}

void AddPackageDialog::initPackageInfo()
{
	PackageDao dao;
	PackageType pkt;
	result_t resp = dao.getPackageType(_package_type_id, &pkt, true);
	if (resp.isOk())
	{
		QString package_name = pkt.name;
		_pkgNameEdit->setText(package_name);
		_pkgNameEdit->setReadOnly(true);
		
		QString pinyin_code = pkt.pinyin;
		_pkgPYCodeEdit->setText(pinyin_code);
		
		int package_category = pkt.category;
		_pkgtypeBox->setCurrentIndex(_pkgtypeBox->findData(package_category));

		QString pack_type = pkt.packType.name;
		_picktypeBox->setCurrentText(pack_type);
		
		int sterilize_type = pkt.sterMethod;
		_stertypeBox->setCurrentIndex(_stertypeBox->findData(sterilize_type));

		int dep_id = pkt.dept.id;
		QString dep_name = pkt.dept.name;
		_deptEdit->setCurrentIdPicked(dep_id, dep_name);
		
		_orders = pkt.detail;
		for (int i = 0; i != _orders.count(); ++i) {
			PackageType::DetailItem it = _orders[i];
			QList<QStandardItem *> items;
			QStandardItem *insItem = new QStandardItem(it.insName);
			insItem->setData(it.insTypeId);
			items << insItem << new QStandardItem(QString::number(it.insNum));
			_model->appendRow(items);
		}
	}
	else
	{
		XNotifier::warn(QString("获取包信息失败: ").append(resp.msg()));
		return;
	}
}

void AddPackageDialog::initInstrumentView() {
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
}

void AddPackageDialog::getOrders()
{
	_orders.clear();

	for (size_t i = 0; i < _model->rowCount(); i++)
	{
		Package::DetailItem pt;
		pt.insName = _model->item(i, 0)->text();
		pt.insTypeId = _model->item(i, 0)->data().toInt();
		pt.insNum = _model->item(i, 1)->text().toInt();
		_orders.append(pt);
	}
}

void AddPackageDialog::addEntry() {
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

void AddPackageDialog::removeEntry() {
	QItemSelectionModel *selModel = _view->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_model->removeRow(indexes.at(i - 1).row());
}

int AddPackageDialog::findRow(int insId) {
	QModelIndexList matches = _model->match(_model->index(0, 0), Qt::UserRole + 1, insId, -1);
	for (const QModelIndex &index : matches) {
		return index.row();
	}
	return -1;
}