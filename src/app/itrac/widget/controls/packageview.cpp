#include "packageview.h"
#include "core/assets.h"
#include "core/itracnamespace.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include "core/constants.h"
#include <xui/images.h>
#include <xui/imageviewer.h>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QFile>

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

QVariantList AbstractPackageView::packages() const {
	QVariantList list;
	for (int i = 0; i != _model->rowCount(); i++) {
		//QString packageId = _model->data(_model->index(i, 0)).toString();
		list.push_back(_model->data(_model->index(i, 0)));
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
	_model->setHeaderData(Implant, Qt::Horizontal, "是否含有植入物");
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
		QStandardItem *insItem = new QStandardItem(resp.getAsBool("ins_count") ? "是" : "否");
		insItem->setData(resp.getAsBool("ins_count") ? QBrush(QColor(255, 160, 122)) : QBrush(QColor(173, 216, 230)), Qt::BackgroundRole);
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
	insItem->setData(implant ? QBrush(QColor(255, 160, 122)) : QBrush(QColor(173, 216, 230)), Qt::BackgroundRole);
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
		insItem->setData(resp.getAsBool("ins_count") ? QBrush(QColor(255, 160, 122)) : QBrush(QColor(173, 216, 230)), Qt::BackgroundRole);
		rowItems << insItem;
		_model->appendRow(rowItems);
	});
}

OrRecyclePackageView::OrRecyclePackageView(QWidget *parent /*= nullptr*/)
	: AbstractPackageView(parent) {
	_model->setColumnCount(VPlate + 1);
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
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
		package_id->setText(id);
		rowItems.append(package_id);

		QStandardItem *package_type_name = new QStandardItem();
		package_type_name->setTextAlignment(Qt::AlignCenter);
		package_type_name->setText(resp.getAsString("package_type_name"));
		package_type_name->setData(resp.getAsString("package_type_id"), 260);
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
		info->setText("请扫描篮筐条码");
		rowItems.append(info);

		_model->appendRow(rowItems);
	});
}

void OrRecyclePackageView::addExtPackage(const QString& pkgId, const QString& pkgTypeId, const QString& pkgName) {
	QList<QStandardItem *> rowItems;
	rowItems.append(new QStandardItem(pkgId));

	QStandardItem *package_type_name = new QStandardItem();
	package_type_name->setTextAlignment(Qt::AlignCenter);
	package_type_name->setText(pkgName);
	package_type_name->setData(pkgTypeId, 260);
	rowItems.append(package_type_name);

	rowItems.append(new QStandardItem(""));
	rowItems.append(new QStandardItem("外来器械"));
	rowItems.append(new QStandardItem(""));
	rowItems.append(new QStandardItem("请扫描篮筐条码"));
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

PackageDetailView::PackageDetailView(QWidget *parent /*= nullptr*/)
	: QWidget(parent)
	, _view(new TableView)
	, _model(new QStandardItemModel(0, 2, _view))
	, _imgLabel(new XPicture(this))
{
	_model->setHeaderData(Name, Qt::Horizontal, "器械名");
	_model->setHeaderData(Number, Qt::Horizontal, "数量");
	_view->setModel(_model);
	_view->setSelectionMode(QAbstractItemView::SingleSelection);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QVBoxLayout *vlayout = new QVBoxLayout(this);
	vlayout->setContentsMargins(0, 0, 0, 0);
	vlayout->setSpacing(0);
	vlayout->addWidget(_imgLabel);
	vlayout->addWidget(_view);

	vlayout->setStretch(1, 1);

	_imgLabel->setFixedHeight(256);
	_imgLabel->setBgColor(QColor(245, 246, 247));
	_imgLabel->setHidden(true);

	connect(_imgLabel, SIGNAL(clicked()), this, SLOT(imgClicked()));
	connect(_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotItemDoubleClicked(const QModelIndex &)));

}

void PackageDetailView::slotItemDoubleClicked(const QModelIndex &index)
{
	int row = index.row();
	QString instrument_id = _view->model()->data(_view->model()->index(row, 0), 257).toString();

	QString fileName = QString("./photo/instrument/%1.png").arg(instrument_id);
	QFile file(fileName);
	if (file.exists()) {
		ImageViewer *viewer = new ImageViewer(fileName);
		viewer->showNormal();
	}
}

void PackageDetailView::loadDetail(const QString& pkgTypeId) {
	QByteArray data("{\"package_type_id\":");
	data.append(pkgTypeId).append('}');
	_http.post(url(PATH_PKGDETAIL_SEARCH), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取包信息: ").append(resp.errorString()));
			return;
		}

		_model->removeRows(0, _model->rowCount());

		QList<QVariant> orders = resp.getAsList("instruments");
		_model->insertRows(0, orders.count());
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();
			_model->setData(_model->index(i, 0), map["instrument_name"]);
			_model->setData(_model->index(i, 0), map["instrument_id"], 257);
			_model->setData(_model->index(i, 1), map["instrument_number"]);
		}

		imgLoad(pkgTypeId);
	});
}

void PackageDetailView::imgLoad(const QString& pkgTypeId)
{
	QString fileName = QString("./photo/package/%1.png").arg(pkgTypeId);
	_imgLabel->setImage(fileName);
	_imgLabel->setHidden(false);
}

void PackageDetailView::clear()
{
	//_imgLabel->setImage();
	_imgLabel->setHidden(true);
	_model->removeRows(0, _model->rowCount());
}

void PackageDetailView::imgClicked()
{
	ImageViewer *viewer = new ImageViewer(_imgLabel->fileName());
	viewer->showMaximized();
}