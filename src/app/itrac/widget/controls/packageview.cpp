#include "packageview.h"
#include "core/assets.h"
#include "core/itracnamespace.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include <QStandardItemModel>

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
	_model->setColumnCount(ExpireDate + 1);
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
	_model->setHeaderData(Department, Qt::Horizontal, "所属科室");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
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
		rowItems << new QStandardItem(id);
		rowItems << new QStandardItem(resp.getAsString("package_type_name"));
		rowItems << new QStandardItem(resp.getAsString("pack_type_name"));
		rowItems << new QStandardItem(resp.getAsString("department_name"));
		rowItems << new QStandardItem(resp.getAsString("valid_date"));
		_model->appendRow(rowItems);
	});
}

SterileCheckPackageView::SterileCheckPackageView(QWidget *parent /*= nullptr*/)
	: TableView(parent), _model(new QStandardItemModel(0, 2, this)) {
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void SterileCheckPackageView::addPackage(const QString &id, const QString &name) {
	QList<QStandardItem*> row;
	row << new QStandardItem(id) << new QStandardItem(name);
	_model->appendRow(row);
}

DispatchPackageView::DispatchPackageView(QWidget *parent /*= nullptr*/)
	: AbstractPackageView(parent) {
	_model->setColumnCount(ExpireDate + 1);
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
	_model->setHeaderData(Department, Qt::Horizontal, "所属科室");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
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

		QList<QStandardItem *> rowItems;
		rowItems << new QStandardItem(id);
		rowItems << new QStandardItem(resp.getAsString("package_type_name"));
		rowItems << new QStandardItem(resp.getAsString("pack_type_name"));
		rowItems << new QStandardItem(resp.getAsString("department_name"));
		rowItems << new QStandardItem(resp.getAsString("valid_date"));
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

		if (resp.getAsString("state") != "R") {
			XNotifier::warn("该包已回收，请勿重复操作");
			return;
		}

		QList<QStandardItem *> rowItems;
		rowItems.append(new QStandardItem(id));
		rowItems << new QStandardItem(resp.getAsString("package_type_name"));
		rowItems << new QStandardItem(resp.getAsString("pack_type_name"));
		rowItems << new QStandardItem(resp.getAsString("from_department_name"));
		rowItems << new QStandardItem(resp.getAsString("valid_date"));
		rowItems.append(new QStandardItem("请扫描篮筐条码"));
		_model->appendRow(rowItems);
	});
}

void OrRecyclePackageView::addExtPackage(const QString& pkgId, const QString& pkgName) {
	QList<QStandardItem *> rowItems;
	rowItems.append(new QStandardItem(pkgId));
	rowItems.append(new QStandardItem(pkgName));
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
