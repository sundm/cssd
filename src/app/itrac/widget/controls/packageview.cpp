#include "packageview.h"
#include "core/assets.h"
#include "core/itracnamespace.h"
#include "core/net/url.h"
#include "ui/inputfields.h"
#include "xnotifier.h"
#include "inliner.h"
#include "core/constants.h"
#include "dialog/registerinstrumentdialog.h"
#include "dialog/addoperationdialog.h"
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
	_model->setColumnCount(Implant + 1);
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
	_model->setHeaderData(Department, Qt::Horizontal, "所属科室");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
	_model->setHeaderData(SterType, Qt::Horizontal, "灭菌类型");
	_model->setHeaderData(Implant, Qt::Horizontal, "是否含有植入物");
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

void SterilePackageView::addPackage(const QString &id) {
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
}

SterileCheckPackageView::SterileCheckPackageView(QWidget *parent /*= nullptr*/)
	: TableView(parent), _model(new QStandardItemModel(0, Implant+1, this)) {
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(Implant, Qt::Horizontal, "是否含有植入物");
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void SterileCheckPackageView::addPackage(const QString &id, const QString &name, const bool &implant) {
	QList<QStandardItem*> row;
	row << new QStandardItem(id) << new QStandardItem(name);
	QStandardItem *insItem = new QStandardItem(implant ? "是" : "否");
	insItem->setData(brushForImport(implant), Qt::BackgroundRole);
	row << insItem;
	_model->appendRow(row);
}

DispatchPackageView::DispatchPackageView(QWidget *parent /*= nullptr*/)
	: AbstractPackageView(parent) {
	_model->setColumnCount(Implant + 1);
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
	_model->setHeaderData(Department, Qt::Horizontal, "所属科室");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
	_model->setHeaderData(Implant, Qt::Horizontal, "是否含有植入物");
}

void DispatchPackageView::addPackage(const QString &id) {
	QString data = QString("{\"package_id\":\"%1\"}").arg(id);
	_http.post(url(PATH_PKG_INFO), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取包信息: ").append(resp.errorString()));
			return;
		}

		if (resp.getAsString("state") != "ST") {
			XNotifier::warn(QString("包 [%1] 尚未完成灭菌审核，或者已发放").arg(id));
			return;
		}
		else if (!resp.getAsBool("sterilize_qualified")) {
			XNotifier::warn(QString("包 [%1] 灭菌不合格，不能对其发放").arg(id));
			return;
		}
		/*
		if (resp.getAsString("state") == "SBT") {
			XNotifier::warn(QString("包 [%1] 尚未完成生物灭菌审核，发放注意风险").arg(id));
			return;
		}
		*/
		if (!(resp.getAsInt("department_id") == Constant::OperatingRoomId || 
			resp.getAsInt("department_id") == Constant::CSSDDeptId)) {
			XNotifier::warn(QString("此包非手术室包，不能对其发放"));
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
		QStandardItem *insItem = new QStandardItem(resp.getAsBool("ins_count") ? "是" : "否");
		insItem->setData(brushForImport(resp.getAsBool("ins_count")), Qt::BackgroundRole);
		rowItems << insItem;
		_model->appendRow(rowItems);
	});
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

OperationInfoTabelView::OperationInfoTabelView(QWidget *parent /*= nullptr*/)
	: TableView(parent), _model(new QStandardItemModel(this))
{
	_model->setColumnCount(PatientName + 1);
	_model->setHeaderData(OperationID, Qt::Horizontal, "手术ID");
	_model->setHeaderData(OperationRoom, Qt::Horizontal, "手术室");
	_model->setHeaderData(OperationTime, Qt::Horizontal, "手术时间");
	_model->setHeaderData(OperationName, Qt::Horizontal, "手术名称");
	_model->setHeaderData(PatientName, Qt::Horizontal, "病人姓名");

	setModel(_model);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));
}

void OperationInfoTabelView::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();
	QString operationId = _model->item(row, OperationID)->text();
	emit operationClicked(operationId);
}

void OperationInfoTabelView::loadOperations()
{
	_model->removeRows(0, _model->rowCount());

	QList<QStandardItem *> rowItems;
	rowItems << new QStandardItem("201911260159");
	rowItems << new QStandardItem("2#手术室");
	rowItems << new QStandardItem("2019-11-22 12:30:00");
	rowItems << new QStandardItem("阑尾手术");
	rowItems << new QStandardItem("王阳");

	for each (QStandardItem * item in rowItems)
	{
		item->setTextAlignment(Qt::AlignCenter);
	}
	_model->appendRow(rowItems);
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

	connect(_view, SIGNAL(operationClicked(const QString &)), this, SIGNAL(operation(const QString &)));
}

void OperationInfoView::addOperation()
{
	AddOperatinDialog d(this);
	if (d.exec() == QDialog::Accepted)
	{
		//todo
	}
}

void OperationInfoView::delOperation()
{

}

void OperationInfoView::refresh()
{

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
}

void OperationPackageView::loadPackages(const QString& operationId)
{
	QList<QStandardItem *> rowItems;
	rowItems << new QStandardItem(operationId);
	rowItems << new QStandardItem("测试器械04");
	rowItems << new QStandardItem("E2009A9050048AF000000213");
	rowItems << new QStandardItem("RFID测试器械包02");

	for each (QStandardItem * item in rowItems)
	{
		item->setTextAlignment(Qt::AlignCenter);
	}
	_model->appendRow(rowItems);
	/*
	QByteArray data("{\"package_type_id\":");
	data.append(",\"card_id\":").append(instrumentID).append('}');
	_http.post(url(PATH_PKGDETAIL_SEARCH), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取包信息: ").append(resp.errorString()));
			return;
		}
		int row = _model->rowCount();
		//todo
		QList<QVariant> orders = resp.getAsList("instruments");
		_model->insertRows(0, orders.count());
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();
			_model->setData(_model->index(i, 0), map["instrument_name"]);
		}
	});
	*/
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
	QList<QStandardItem *> rowItems;
	rowItems << new QStandardItem(instrumentID);
	rowItems << new QStandardItem("测试器械04");
	rowItems << new QStandardItem("E2009A9050048AF000000213");
	rowItems << new QStandardItem("RFID测试器械包02");

	for each (QStandardItem * item in rowItems)
	{
		item->setTextAlignment(Qt::AlignCenter);
	}
	_model->appendRow(rowItems);
	/*
	QByteArray data("{\"package_type_id\":");
	data.append(",\"card_id\":").append(instrumentID).append('}');
	_http.post(url(PATH_PKGDETAIL_SEARCH), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取包信息: ").append(resp.errorString()));
			return;
		}
		int row = _model->rowCount();
		//todo
		QList<QVariant> orders = resp.getAsList("instruments");
		_model->insertRows(0, orders.count());
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();
			_model->setData(_model->index(i, 0), map["instrument_name"]);
		}
	});
	*/
}

PackageDetailView::PackageDetailView(QWidget *parent /*= nullptr*/)
	: TableView(parent)
	, _model(new QStandardItemModel(this))
	, _instruments(new QList<instrument_struct>)
{
	_model->setColumnCount(Tips + 1);
	_model->setHeaderData(Name, Qt::Horizontal, "器械名称");
	_model->setHeaderData(Total, Qt::Horizontal, "数量");
	_model->setHeaderData(Scanned, Qt::Horizontal, "通过");
	_model->setHeaderData(Residue, Qt::Horizontal, "剩余");
	_model->setHeaderData(Tips, Qt::Horizontal, "说明");

	setModel(_model);

	horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
	horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
	horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
	setColumnWidth(1, 100);
	setColumnWidth(2, 100);
	setColumnWidth(3, 100);
	setColumnWidth(4, 200);

	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	//connect(_imgLabel, SIGNAL(clicked()), this, SLOT(imgClicked()));
	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotItemClicked(const QModelIndex &)));
	//connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint&)));

}

void PackageDetailView::slotItemClicked(const QModelIndex &index)
{
	int row = index.row();
	QString instrument_id = _model->data(_model->index(row, 0), 257).toString();

	QString fileName = QString("./photo/instrument/%1.png").arg(instrument_id);
	QFile file(fileName);
	if (file.exists()) {
		//todo
	}
}

void PackageDetailView::updateState(int pkg_record, int ins_state)
{
	if (posIndex.isValid()) {
		QModelIndex stateIdx = posIndex.sibling(posIndex.row(), 2);
		_model->setData(stateIdx, ins_state, 257);
		_model->setData(stateIdx, literalSteType(ins_state));
		_model->setData(stateIdx, brushForSteType(ins_state), Qt::BackgroundRole);

		emit sendData(pkg_record);
	}

}

void PackageDetailView::scanned(const QString & code) {
	QList<instrument_struct>::const_iterator k;
	int i = 0;
	for (k = _instruments->constBegin(); k != _instruments->constEnd(); k++)
	{
		if (k->codes.contains(code))
		{
			instrument_struct st = _instruments->at(i);
			st.codes.removeOne(code);
			_instruments->replace(i, st);
			
			int count = _model->item(i, 2)->text().toInt() + 1;
			_model->item(i, 2)->setText(QString::number(count));

			count = _model->item(i, 3)->text().toInt() - 1;
			_model->item(i, 3)->setText(QString::number(count));

			if (0 == count)
			{
				_model->item(i, 4)->setText("通过检查");
			}
			break;
		}
		i++;
	}
	
}

void PackageDetailView::loadDetail(const QHash<QString, QString> * const maps) {
	_instruments->clear();
	QStringList names = maps->values();
	names.removeDuplicates();

	for each (QString name in names)
	{
		QStringList codelist;
		QHash<QString, QString>::const_iterator j;
		for (j = maps->constBegin(); j != maps->constEnd(); j++)
		{
			if (j.value() == name)
			{
				codelist.append(j.key());
			}
		}

		instrument_struct ins;
		ins.name = name;
		ins.codes = codelist;
		_instruments->append(ins);
	}

	QList<instrument_struct>::const_iterator k;
	for (k = _instruments->constBegin(); k != _instruments->constEnd(); k++)
	{
		QList<QStandardItem *> rowItems;
		rowItems << new QStandardItem(k->name);
		rowItems << new QStandardItem(QString::number(k->codes.size()));
		rowItems << new QStandardItem("0");
		rowItems << new QStandardItem(QString::number(k->codes.size()));
		rowItems << new QStandardItem();
		for each (QStandardItem * item in rowItems)
		{
			item->setTextAlignment(Qt::AlignCenter);
		}
		_model->appendRow(rowItems);
	}
	
}

//void PackageDetailView::imgLoad(const QString& pkgTypeId)
//{
//	QString fileName = QString("./photo/package/%1.png").arg(pkgTypeId);
//	_imgLabel->setImage(fileName);
//	_imgLabel->setHidden(false);
//}

void PackageDetailView::clear()
{
	//_imgLabel->setImage();
	//_imgLabel->setHidden(true);
	_model->removeRows(0, _model->rowCount());
}

//void PackageDetailView::imgClicked()
//{
//	ImageViewer *viewer = new ImageViewer(_imgLabel->fileName());
//	viewer->showMaximized();
//}