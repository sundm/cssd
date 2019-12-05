#include "packageview.h"
#include "core/assets.h"
#include "core/itracnamespace.h"
#include "core/net/url.h"
#include "ui/inputfields.h"
#include "xnotifier.h"
#include "model/itemdelegate.h"
#include "inliner.h"
#include "core/constants.h"
#include "dialog/registerinstrumentdialog.h"
#include "dialog/addoperationdialog.h"
#include "rdao/dao/PackageDao.h"
#include "rdao/dao/surgerydao.h"
#include "rdao/entity/surgery.h"
#include "rdao/entity/operator.h"
#include "rdao/dao/instrumentdao.h"
#include <xui/images.h>
#include <xui/imageviewer.h>
#include <QtWidgets/QtWidgets>
#include <QLabel>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QFile>
#include <QMenu>
#include <QHeaderView>

AbstractPackageView::AbstractPackageView(QWidget *parent)
	: TableView(parent), _model(new QStandardItemModel(this))
{
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
}

bool AbstractPackageView::hasPackage(const QString &id) const
{
	QModelIndexList matches = _model->match(_model->index(0, 0), Qt::DisplayRole, id, 1);
	return !matches.isEmpty();
}

bool AbstractPackageView::hasImplantPackage() const
{
	bool b = false;
	for (int i = 0; i != _model->rowCount(); i++) {
		if (_model->data(_model->index(i, 0), Qt::UserRole + 1).toBool())
		{
			b = true;
			break;
		}
	}
	return b;
}

QVariantList AbstractPackageView::packageIds() const {
	QVariantList list;
	for (int i = 0; i != _model->rowCount(); i++) {
		QString packageId = _model->data(_model->index(i, 0)).toString();
		if (packageId.isEmpty())
			packageId = "0";
		list.push_back(packageId);
	}
	return list;
}

QVariantList AbstractPackageView::cardIds() const {
	QVariantList list;
	for (int i = 0; i != _model->rowCount(); i++) {
		QString cardId = _model->data(_model->index(i, 1)).toString();
		if (cardId.isEmpty())
			cardId = "0";
		list.push_back(cardId);
	}
	return list;
}

SterilePackageView::SterilePackageView(QWidget *parent /*= nullptr*/)
	: AbstractPackageView(parent) {
	_pkgList.clear();
	_model->setColumnCount(Implant + 1);
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
	_model->setHeaderData(Department, Qt::Horizontal, "所属科室");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
	_model->setHeaderData(SterType, Qt::Horizontal, "灭菌类型");
	_model->setHeaderData(Implant, Qt::Horizontal, "是否含有植入物");
	
	horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	setColumnWidth(0, 400);
}

bool SterilePackageView::matchType(int type) const
{
	bool b = true;
	for (int i = 0; i != _model->rowCount(); i++) {
		int st = _model->data(_model->index(i, SterType), Qt::UserRole + 1).toInt();
		if (st == 0 || type == st)
			continue;
		else
			b = false;
	}
	return b;
}

const QList<Package> &SterilePackageView::packages() const {
	return _pkgList;
}

void SterilePackageView::clear() {
	_model->removeRows(0, _model->rowCount());
	_pkgList.clear();
}

void SterilePackageView::addPackage(const QString &id) {
	for each (Package pkg in _pkgList)
	{
		if (id.compare(pkg.udi) == 0)
		{
			//XNotifier::warn(QString("该器械包已经添加:").append(id), -1);
			return;
		}
	}

	PackageDao dao;
	Package pkg;

	result_t resp = dao.getPackage(id, &pkg);
	if (resp.isOk())
	{
		if (pkg.status == Rt::FlowStatus::Sterilized)
		{
			XNotifier::warn(QString("包 [%1] 已进行过灭菌，请勿重复登记").arg(id));
			return;
		}

		_pkgList.append(pkg);

		QList<QStandardItem *> rowItems;
		QStandardItem *idItem = new QStandardItem(pkg.udi);
		//idItem->setData(resp.getAsBool("ins_count"));
		rowItems << idItem;
		rowItems << new QStandardItem(pkg.name);
		rowItems << new QStandardItem(pkg.packType.name);
		rowItems << new QStandardItem(pkg.dept.name);
		rowItems << new QStandardItem();//todo 

		QStandardItem *typeItem = new QStandardItem(literal_sterile_type(pkg.sterMethod));
		typeItem->setData(pkg.sterMethod);
		rowItems << typeItem;

		QStandardItem *insItem = new QStandardItem(false ? "是" : "否");
		insItem->setData(brushForImport(false), Qt::BackgroundRole);
		rowItems << insItem;
		_model->appendRow(rowItems);
	}
	else
	{
		XNotifier::warn(QString("无法获取包信息: ").append(resp.msg()));
		return;
	}
	/*
	QString data = QString("{\"package_id\":\"%1\"}").arg(id);
	_http.post(url(PATH_PKG_INFO), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取包信息: ").append(resp.errorString()));
			return;
		}

		if (resp.getAsString("state") != "P") {
			XNotifier::warn(QString("包 [%1] 已进行过灭菌，请勿重复登记").arg(id));
			return;
		}
		QList<QStandardItem *> rowItems;
		QStandardItem *idItem = new QStandardItem(id);
		idItem->setData(resp.getAsBool("ins_count"));
		rowItems << idItem;
		rowItems << new QStandardItem(resp.getAsString("package_type_name"));
		rowItems << new QStandardItem(resp.getAsString("pack_type_name"));
		rowItems << new QStandardItem(resp.getAsString("department_name"));
		rowItems << new QStandardItem(resp.getAsString("valid_date"));

		QStandardItem *typeItem = new QStandardItem(literal_sterile_type(resp.getAsInt("sterilize_type")));
		typeItem->setData(resp.getAsInt("sterilize_type"));
		rowItems << typeItem;

		QStandardItem *insItem = new QStandardItem(resp.getAsBool("ins_count") ? "是" : "否");
		insItem->setData(brushForImport(resp.getAsBool("ins_count")), Qt::BackgroundRole);
		rowItems << insItem;
		_model->appendRow(rowItems);
		
	});
	*/
}

SterileCheckPackageView::SterileCheckPackageView(QWidget *parent /*= nullptr*/)
	: TableView(parent), _model(new QStandardItemModel(0, Wet + 1, this)) {
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(Implant, Qt::Horizontal, "是否含有植入物");
	_model->setHeaderData(Wet, Qt::Horizontal, "是否发生湿包");
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	setColumnWidth(0, 400);
}

const QList<SterilizeResult::PackageItem> & SterileCheckPackageView::getPackages()
{
	return _packages;
}

void SterileCheckPackageView::itemChecked(const QModelIndex & index, const bool checked)
{
	SterilizeResult::PackageItem item = _packages.at(index.row());
	item.isWetPack = checked;
	_packages.replace(index.row(), item);
}

void SterileCheckPackageView::addPackages(const QList<SterilizeResult::PackageItem>& pkgs, const bool readOnly) {
	_packages = pkgs;
	_model->removeRows(0, _model->rowCount());
	_model->insertRows(0, pkgs.count());
	for (int i = 0; i != pkgs.count(); ++i) {
		SterilizeResult::PackageItem item = pkgs[i];
		_model->setData(_model->index(i, 0), item.udi);
		_model->setData(_model->index(i, 1), item.name);
		_model->setData(_model->index(i, 2), "");//todo
		CheckBoxDelegate *checkBox = new CheckBoxDelegate;
		connect(checkBox, SIGNAL(setChecked(const QModelIndex &, const bool)), this, SLOT(itemChecked(const QModelIndex &, const bool)));
		checkBox->setColumn(3, readOnly);
		setItemDelegateForColumn(3, checkBox);
		_model->setData(_model->index(i, 3), item.isWetPack, Qt::DisplayRole);
	}
}

DispatchPackageView::DispatchPackageView(QWidget *parent /*= nullptr*/)
	: AbstractPackageView(parent) {
	_pkgList.clear();
	_model->setColumnCount(Implant + 1);
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
	_model->setHeaderData(Department, Qt::Horizontal, "所属科室");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
	_model->setHeaderData(Implant, Qt::Horizontal, "是否含有植入物");

	horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	setColumnWidth(0, 400);
}

const QList<Package> &DispatchPackageView::packages() const {
	return _pkgList;
}

void DispatchPackageView::clear() {
	_model->removeRows(0, _model->rowCount());
	_pkgList.clear();
}

void DispatchPackageView::addPackage(const QString &id) {
	for each (Package pkg in _pkgList)
	{
		if (id.compare(pkg.udi) == 0)
		{
			//XNotifier::warn(QString("该器械包已经添加:").append(id), -1);
			return;
		}
	}

	PackageDao dao;
	Package pkg;

	result_t resp = dao.getPackage(id, &pkg);
	if (resp.isOk())
	{
		if (pkg.status == Rt::FlowStatus::Dispatched)
		{
			XNotifier::warn(QString("包 [%1] 已进行过发放，请勿重复登记").arg(id));
			return;
		}
		_pkgList.append(pkg);

		QList<QStandardItem *> rowItems;
		QStandardItem *idItem = new QStandardItem(pkg.udi);
		//idItem->setData(resp.getAsBool("ins_count"));
		rowItems << idItem;
		rowItems << new QStandardItem(pkg.name);
		rowItems << new QStandardItem(pkg.packType.name);
		rowItems << new QStandardItem(pkg.dept.name);
		rowItems << new QStandardItem();//todo 

		QStandardItem *insItem = new QStandardItem(false ? "是" : "否");
		insItem->setData(brushForImport(false), Qt::BackgroundRole);
		rowItems << insItem;
		_model->appendRow(rowItems);
	}
	else
	{
		XNotifier::warn(QString("无法获取包信息: ").append(resp.msg()));
		return;
	}
}

OrRecyclePackageView::OrRecyclePackageView(QWidget *parent /*= nullptr*/)
	: AbstractPackageView(parent) {
	_model->setColumnCount(VPlate + 1);
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(PkgCode, Qt::Horizontal, "包内器械清单号");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
	_model->setHeaderData(Department, Qt::Horizontal, "来源科室");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
	_model->setHeaderData(VPlate, Qt::Horizontal, "装篮");
}

void OrRecyclePackageView::addPackage(const QString &id) {
	QString data = QString("{\"package_id\":\"%1\"}").arg(id);
	_http.post(url(PATH_PKG_INFO), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取包信息: ").append(resp.errorString()));
			return;
		}

		if (resp.getAsString("state") == "R") {
			XNotifier::warn("该包已回收，请勿重复操作");
			return;
		}

		QList<QStandardItem *> rowItems;
		
		QStandardItem *package_id = new QStandardItem();
		package_id->setTextAlignment(Qt::AlignCenter);
		package_id->setText(resp.getAsString("pkg_id"));
		rowItems.append(package_id);

		QStandardItem *package_code = new QStandardItem();
		package_code->setTextAlignment(Qt::AlignCenter);
		package_code->setText(resp.getAsString("card_id"));
		rowItems.append(package_code);

		QStandardItem *package_type_name = new QStandardItem();
		package_type_name->setTextAlignment(Qt::AlignCenter);
		package_type_name->setText(resp.getAsString("package_type_name"));
		package_type_name->setData(resp.getAsString("package_type_id"), 260);
		int steType = resp.getAsInt("card_record");
		package_type_name->setData(brushForSteType(steType), Qt::BackgroundRole);
		rowItems.append(package_type_name);

		QStandardItem *pack_type_name = new QStandardItem();
		pack_type_name->setTextAlignment(Qt::AlignCenter);
		pack_type_name->setText(resp.getAsString("pack_type_name"));
		rowItems.append(pack_type_name);

		QStandardItem *from_department_name = new QStandardItem();
		from_department_name->setTextAlignment(Qt::AlignCenter);
		from_department_name->setText(resp.getAsString("from_department_name"));
		rowItems.append(from_department_name);

		QStandardItem *valid_date = new QStandardItem();
		valid_date->setTextAlignment(Qt::AlignCenter);
		valid_date->setText(resp.getAsString("valid_date"));
		rowItems.append(valid_date);

		QStandardItem *info = new QStandardItem();
		info->setTextAlignment(Qt::AlignCenter);
		info->setText("请扫描网篮条码");
		rowItems.append(info);

		_model->appendRow(rowItems);
	});
}

void OrRecyclePackageView::addExtPackage(const QString& pkgId, const QString& pkgTypeId, const QString& pkgName) {
	QList<QStandardItem *> rowItems;
	rowItems.append(new QStandardItem(pkgId));
	rowItems.append(new QStandardItem("0"));

	QStandardItem *package_type_name = new QStandardItem();
	package_type_name->setTextAlignment(Qt::AlignCenter);
	package_type_name->setText(pkgName);
	package_type_name->setData(pkgTypeId, 260);
	rowItems.append(package_type_name);

	rowItems.append(new QStandardItem(""));
	rowItems.append(new QStandardItem("外来器械"));
	rowItems.append(new QStandardItem(""));
	rowItems.append(new QStandardItem("请扫描网篮条码"));
	_model->appendRow(rowItems);
}

void OrRecyclePackageView::updatePlate(const QString &plateId)
{
	QByteArray data("{\"plate_id\":");
	data.append(plateId).append('}');
	_http.post(url(PATH_PLATE_SEARCH), data, [plateId, this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取编号[%1]的网篮信息").arg(plateId));
			return;
		}

		QList<QVariant> plates = resp.getAsList("plates");
		if (plates.isEmpty()) {
			XNotifier::warn(QString("编号[%1]的网篮不在系统资产目录中").arg(plateId));
			return;
		}

		QVariantMap map = plates[0].toMap();
		bool idle = "1" == map["is_finished"].toString();

		if (!idle) {
			XNotifier::warn(QString("网篮<%1>正在使用，无法添加").arg(plateId));
			return;
		}

		for (int i = 0; i != _model->rowCount(); ++i) {
			QStandardItem *item = _model->item(i, OrRecyclePackageView::VPlate);
			item->setText(map["plate_name"].toString());
			item->setData(plateId.toInt());
		}
	});
}

int OrRecyclePackageView::plate() const
{
	return _model->item(0, OrRecyclePackageView::VPlate)->data().toInt();
}

PackageSimpleInfoView::PackageSimpleInfoView(QWidget *parent /*= nullptr*/)
	: QWidget(parent)
	, _totalNumLabel(new QLabel)
	, _scannedNumLabel(new QLabel)
	, _residueNumLabel(new QLabel)
	, _unusualNumLabel(new QLabel)
{
	const QString m_label_style = "min-width: 48px; min-height: 48px;max-width:48px; max-height: 48px;border-radius: 24px;";
	_totalNumLabel->setStyleSheet(QString("%1%2").arg(m_label_style).arg("background:grey"));
	_scannedNumLabel->setStyleSheet(QString("%1%2").arg(m_label_style).arg("background:green"));
	_residueNumLabel->setStyleSheet(QString("%1%2").arg(m_label_style).arg("background:yellow"));
	_unusualNumLabel->setStyleSheet(QString("%1%2").arg(m_label_style).arg("background:red"));

	QFont font1;
	font1.setPointSize(16);
	_totalNumLabel->setFont(font1);
	_scannedNumLabel->setFont(font1);
	_residueNumLabel->setFont(font1);
	_unusualNumLabel->setFont(font1);

	_totalNumLabel->setAlignment(Qt::AlignCenter);
	_scannedNumLabel->setAlignment(Qt::AlignCenter);
	_residueNumLabel->setAlignment(Qt::AlignCenter);
	_unusualNumLabel->setAlignment(Qt::AlignCenter);

	_totalNumLabel->setText("0");
	_scannedNumLabel->setText("0");
	_residueNumLabel->setText("0");
	_unusualNumLabel->setText("0");

	QHBoxLayout *hLayout = new QHBoxLayout(this);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(_totalNumLabel);
	hLayout->addWidget(_scannedNumLabel);
	hLayout->addWidget(_residueNumLabel);
	hLayout->addWidget(_unusualNumLabel);
	hLayout->addStretch(0);
}

void PackageSimpleInfoView::updatePackageInfo(const int &insCount)
{
	_totalNum = insCount;
	_scannedNum = 0;
	_unusualNum = 0;

	_totalNumLabel->setText(QString::number(_totalNum));
	_scannedNumLabel->setText(QString::number(_scannedNum));
	_residueNumLabel->setText(QString::number(_totalNum));
	_unusualNumLabel->setText(QString::number(_unusualNum));
}

void PackageSimpleInfoView::scanned()
{
	//todo
	_scannedNum++;
	_scannedNumLabel->setText(QString::number(_scannedNum));
	_residueNumLabel->setText(QString::number(_totalNum - _scannedNum));
}

void PackageSimpleInfoView::unusualed()
{
	//todo
	_unusualNum++;
	_unusualNumLabel->setText(QString::number(_unusualNum));
}

PackageInfoView::PackageInfoView(QWidget *parent /*= nullptr*/)
	: QWidget(parent)
	, _packageIDLabel(new QLabel)
	, _tipsLabel(new QLabel)
	, _packageNameLabel(new QLabel)
	, _totalNumLabel(new QLabel)
	, _scannedNumLabel(new QLabel)
	, _residueNumLabel(new QLabel)
	, _unusualNumLabel(new QLabel)
{
	const QString m_label_style = "min-width: 48px; min-height: 48px;max-width:48px; max-height: 48px;border-radius: 24px;";
	_totalNumLabel->setStyleSheet(QString("%1%2").arg(m_label_style).arg("background:grey"));
	_scannedNumLabel->setStyleSheet(QString("%1%2").arg(m_label_style).arg("background:green"));
	_residueNumLabel->setStyleSheet(QString("%1%2").arg(m_label_style).arg("background:yellow"));
	_unusualNumLabel->setStyleSheet(QString("%1%2").arg(m_label_style).arg("background:red"));

	QFont font1;
	font1.setPointSize(16);
	_totalNumLabel->setFont(font1);
	_scannedNumLabel->setFont(font1);
	_residueNumLabel->setFont(font1);
	_unusualNumLabel->setFont(font1);

	_totalNumLabel->setAlignment(Qt::AlignCenter);
	_scannedNumLabel->setAlignment(Qt::AlignCenter);
	_residueNumLabel->setAlignment(Qt::AlignCenter);
	_unusualNumLabel->setAlignment(Qt::AlignCenter);

	_totalNumLabel->setText("0");
	_scannedNumLabel->setText("0");
	_residueNumLabel->setText("0");
	_unusualNumLabel->setText("0");

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(_totalNumLabel);
	hLayout->addWidget(_scannedNumLabel);
	hLayout->addWidget(_residueNumLabel);
	hLayout->addWidget(_unusualNumLabel);

	QFont font;
	font.setPointSize(20);

	QHBoxLayout *bLayout = new QHBoxLayout;
	QLabel *idlabel = new QLabel(QString("包编号:"));
	idlabel->setFont(font);
	bLayout->addWidget(idlabel);

	_packageIDLabel->setMinimumWidth(400);
	_packageIDLabel->setFont(font);
	bLayout->addWidget(_packageIDLabel);

	QLabel *namelabel = new QLabel(QString("包名称:"));
	namelabel->setFont(font);
	bLayout->addWidget(namelabel);

	_packageNameLabel->setFont(font);
	bLayout->addWidget(_packageNameLabel);
	bLayout->addLayout(hLayout);
	bLayout->setStretch(3, 1);


	_tipsLabel->setFont(font);
	_tipsLabel->setText(QString("请扫描篮筐ID"));

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->addWidget(_tipsLabel);
	vLayout->addLayout(bLayout);
}

void PackageInfoView::updateTips(const QString& tips)
{
	_tipsLabel->setText(tips);
}

void PackageInfoView::updatePackageInfo(const QString &pkgId, const QString &pkgName, const int &insCount)
{
	_packageIDLabel->setText(pkgId);
	_packageNameLabel->setText(pkgName);

	_totalNum = insCount;
	_scannedNum = 0;
	_unusualNum = 0;

	_totalNumLabel->setText(QString::number(_totalNum));
	_scannedNumLabel->setText(QString::number(_scannedNum));
	_residueNumLabel->setText(QString::number(_totalNum));
	_unusualNumLabel->setText(QString::number(_unusualNum));

	_tipsLabel->setText("请扫描器械");
}

void PackageInfoView::scanned()
{
	//todo
	_scannedNum++;
	_scannedNumLabel->setText(QString::number(_scannedNum));
	_residueNumLabel->setText(QString::number(_totalNum - _scannedNum));
}

void PackageInfoView::unusualed()
{
	//todo
	_unusualNum++;
	_unusualNumLabel->setText(QString::number(_unusualNum));
}

bool PackageInfoView::isScanFinished()
{
	if (_totalNum == _scannedNum && _unusualNum == 0)
		return true;
	else
		return false;
}

void PackageInfoView::reset()
{
	_packageIDLabel->setText("");
	_packageNameLabel->setText("");
	_tipsLabel->setText(QString("请扫描篮筐ID"));

	_totalNumLabel->setText("0");
	_scannedNumLabel->setText("0");
	_residueNumLabel->setText("0");
	_unusualNumLabel->setText("0");

	_totalNum = 0;
	_scannedNum = 0;
	_unusualNum = 0;
}

OperationInfoTabelView::OperationInfoTabelView(QWidget *parent /*= nullptr*/)
	: TableView(parent), _model(new QStandardItemModel(this))
{
	_model->setColumnCount(PatientName + 1);
	_model->setHeaderData(OperationID, Qt::Horizontal, "手术ID");
	_model->setHeaderData(OperationRoom, Qt::Horizontal, "手术室");
	_model->setHeaderData(OperationTime, Qt::Horizontal, "手术时间");
	_model->setHeaderData(OperationName, Qt::Horizontal, "手术名称");
	_model->setHeaderData(PatientId, Qt::Horizontal, "病人ID");
	_model->setHeaderData(PatientName, Qt::Horizontal, "病人姓名");

	setModel(_model);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));
}

void OperationInfoTabelView::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();
	Surgery surgery = _surgeries.at(row);
	emit operationClicked(surgery.id);
}

void OperationInfoTabelView::loadSurgeries()
{
	SurgeryDao dao;

	result_t resp = dao.getSurgeryList(Rt::SurgeryStatus::WaitingForCheck, &_surgeries);
	if (resp.isOk())
	{
		_model->removeRows(0, _model->rowCount());

		for each (Surgery item in _surgeries)
		{
			QList<QStandardItem *> rowItems;
			rowItems << new QStandardItem(QString::number(item.id));
			rowItems << new QStandardItem(item.surgeryRoom);
			rowItems << new QStandardItem(item.surgeryTime.toString("yyyy-MM-dd HH:mm:ss"));
			rowItems << new QStandardItem(item.surgeryName);
			rowItems << new QStandardItem(QString::number(item.patientId));
			rowItems << new QStandardItem(item.patientName);

			for each (QStandardItem * item in rowItems)
			{
				item->setTextAlignment(Qt::AlignCenter);
			}
			_model->appendRow(rowItems);
		}
	}
	else
	{
		XNotifier::warn(QString("获取手术列表失败: ").append(resp.msg()));
	}

	

}

OperationInfoView::OperationInfoView(QWidget *parent /*= nullptr*/)
	: QWidget(parent), _view(new OperationInfoTabelView(this))
{
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);

	QToolButton *addButton = new QToolButton;
	addButton->setIcon(QIcon(":/res/plus-24.png"));
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addOperation()));

	QToolButton *minusButton = new QToolButton;
	minusButton->setIcon(QIcon(":/res/delete-24.png"));
	minusButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(delOperation()));

	QToolButton *refreshButton = new QToolButton;
	refreshButton->setIcon(QIcon(":/res/refresh-24.png"));
	refreshButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(refreshButton);
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(refresh()));

	hLayout->addStretch(0);

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->addLayout(hLayout);
	vLayout->addWidget(_view);

	connect(_view, SIGNAL(operationClicked(const int)), this, SIGNAL(operation(const int)));
}

void OperationInfoView::loadSurgeries()
{
	_view->loadSurgeries();
}

void OperationInfoView::addOperation()
{
	AddOperatinDialog d(this);
	if (d.exec() == QDialog::Accepted)
	{
		refresh();
	}
}

void OperationInfoView::delOperation()
{

}

void OperationInfoView::refresh()
{
	_view->loadSurgeries();
}

OperationPackageView::OperationPackageView(QWidget *parent /*= nullptr*/)
	: TableView(parent), _model(new QStandardItemModel(this))
{
	_model->setColumnCount(State + 1);
	_model->setHeaderData(PackageType, Qt::Horizontal, "包类型");
	_model->setHeaderData(PackageID, Qt::Horizontal, "包ID");
	_model->setHeaderData(PackageName, Qt::Horizontal, "包名称");
	_model->setHeaderData(State, Qt::Horizontal, "状态");

	setModel(_model);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));

}

void OperationPackageView::slotRowDoubleClicked(const QModelIndex &index)
{
	if (_model->rowCount() != _packages.count())
	{
		return;
	}

	int row = index.row();
	bool isExisted = _model->item(row, 0)->data(Qt::UserRole + 2).toBool();
	if (isExisted)
	{
		QString pkgUdi = _model->item(row, 1)->data(Qt::DisplayRole).toString();

		for each (Package pkg in _packages)
		{
			if (pkg.udi.compare(pkgUdi) == 0)
			{
				emit packageClicked(pkg);
			}
		}
	}
}

bool OperationPackageView::isFinished()
{
	return true;
}

bool OperationPackageView::addPackage(const Package &pkg)
{
	for each (Package p in _packages)
	{
		if (p.udi == pkg.udi) return false;
	}

	for (int i = 0; i < _model->rowCount(); i++)
	{
		int pkg_type_id = _model->item(i, 0)->data(Qt::UserRole + 1).toInt();
		bool isExisted = _model->item(i, 0)->data(Qt::UserRole + 2).toBool();

		if (isExisted) continue;

		if (pkg_type_id == pkg.typeId)
		{
			_model->setData(_model->index(i, 0), true, Qt::UserRole + 2);
			_model->setData(_model->index(i, 1), pkg.udi, Qt::DisplayRole);
			_model->setData(_model->index(i, 2), pkg.name, Qt::DisplayRole);
			_model->setData(_model->index(i, 3), QString("已绑定"), Qt::DisplayRole);
			_packages.append(pkg);
			return true;
		}
	}

	return false;
}

void OperationPackageView::loadPackages(const int surgeryId)
{
	_packages.clear();

	SurgeryDao dao;
	result_t resp = dao.getSurgery(surgeryId, &_surgery);
	if (resp.isOk())
	{
		_model->removeRows(0, _model->rowCount());
		QList<Surgery::DetailItem> details = _surgery.detail;
		for each (Surgery::DetailItem item in details)
		{
			for (int i = 0; i < item.pkgNum; i++)
			{
				QList<QStandardItem *> rowItems;
				QStandardItem *typeItem = new QStandardItem(item.pkgTypeName);
				typeItem->setData(item.pkgTypeId, Qt::UserRole + 1);
				typeItem->setData(false, Qt::UserRole + 2);
				rowItems << typeItem;
				_model->appendRow(rowItems);
			}

		}
	}
	else
	{
		XNotifier::warn(QString("编号[%1]的手术无法获取器械包列表").arg(surgeryId));
		return;
	}
	
}

UnusualInstrumentView::UnusualInstrumentView(QWidget *parent /*= nullptr*/)
	: TableView(parent), _model(new QStandardItemModel(this))
{
	_model->setColumnCount(PackageName + 1);
	_model->setHeaderData(InstrumentID, Qt::Horizontal, "异常器械ID");
	_model->setHeaderData(InstrumentName, Qt::Horizontal, "异常器械名");
	_model->setHeaderData(PackageID, Qt::Horizontal, "所属包ID");
	_model->setHeaderData(PackageName, Qt::Horizontal, "所属包名");

	setModel(_model);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void UnusualInstrumentView::addUnusual(const QString& instrumentID)
{
	InstrumentDao dao;
	Instrument it;
	result_t resp = dao.getInstrument(instrumentID, &it);
	if (resp.isOk())
	{
		PackageDao dao;
		Package pkg;
		dao.getPackage(it.packageUdi, &pkg);
		QList<QStandardItem *> rowItems;
		rowItems << new QStandardItem(it.udi);
		rowItems << new QStandardItem(it.name);
		rowItems << new QStandardItem(it.packageUdi);
		rowItems << new QStandardItem(pkg.name);

		for each (QStandardItem * item in rowItems)
		{
			item->setTextAlignment(Qt::AlignCenter);
		}
		_model->appendRow(rowItems);
	}
}

void UnusualInstrumentView::reset()
{
	_model->removeRows(0, _model->rowCount());
}

PackageDetailView::PackageDetailView(QWidget *parent /*= nullptr*/)
	: TableView(parent)
	, _model(new QStandardItemModel(this))
	, _instruments(new QList<Instrument>)
{
	_model->setColumnCount(Status + 1);
	_model->setHeaderData(Name, Qt::Horizontal, "器械名称");
	_model->setHeaderData(Code, Qt::Horizontal, "器械UDI");
	_model->setHeaderData(Status, Qt::Horizontal, "状态");

	setModel(_model);

	//horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	//horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	//horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
	//setColumnWidth(0, 200);
	//setColumnWidth(1, 300);
	//setColumnWidth(2, 300);

	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	//connect(_imgLabel, SIGNAL(clicked()), this, SLOT(imgClicked()));
	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotItemClicked(const QModelIndex &)));
	//connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint&)));

}

void PackageDetailView::slotItemClicked(const QModelIndex &index)
{
	int row = index.row();
	QString instrument_id = _model->item(row, 1)->text();
	emit onclick(instrument_id);
}

void PackageDetailView::scanned(const QString & code) {
	QList<Instrument>::const_iterator k;
	int i = 0;
	bool iScanned = false;
	for (k = _instruments->constBegin(); k != _instruments->constEnd(); k++)
	{
		if (k->udi.compare(code) == 0)
		{	
			_model->item(i, 2)->setText("通过检查");
			iScanned = true;
			break;
		}
		i++;
	}

	if (iScanned)
		emit scand(code);
	else
		emit unusual(code);
}

void PackageDetailView::loadDetail(const QList<Instrument> * instruments) {
	_instruments->clear();
	_model->removeRows(0, _model->rowCount());

	QList<Instrument>::const_iterator k;
	for (k = instruments->constBegin(); k != instruments->constEnd(); k++)
	{
		QList<QStandardItem *> rowItems;
		rowItems << new QStandardItem(k->name);
		rowItems << new QStandardItem(k->udi);
		rowItems << new QStandardItem("");
		for each (QStandardItem * item in rowItems)
		{
			item->setTextAlignment(Qt::AlignCenter);
		}
		_model->appendRow(rowItems);
		_instruments->append(*k);
	}
	
}

void PackageDetailView::reset()
{
	_model->removeRows(0, _model->rowCount());
	_instruments->clear();
}

//void PackageDetailView::imgClicked()
//{
//	ImageViewer *viewer = new ImageViewer(_imgLabel->fileName());
//	viewer->showMaximized();
//}