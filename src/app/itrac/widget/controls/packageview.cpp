#include "packageview.h"
#include "core/assets.h"
#include "core/itracnamespace.h"
//#include "core/net/url.h"
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
	Package::fetchOnce(id, [this, id](Package *pkg) {
		if (pkg->state != itrac::Packed) {
			XNotifier::warn(QString("包 [%1] 已进行过灭菌，请勿重复登记").arg(id));
			return;
		}
		QList<QStandardItem *> rowItems;
		rowItems << new QStandardItem(id);
		rowItems << new QStandardItem(pkg->name);
		rowItems << new QStandardItem(pkg->packName);
		rowItems << new QStandardItem(pkg->deparment);
		rowItems << new QStandardItem(pkg->expireDate);
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
	Package::fetchOnce(id, [this, id](Package *pkg) {
		if (pkg->state != itrac::SteExamined) {
			XNotifier::warn(QString("包 [%1] 尚未完成灭菌审核，或者已发放").arg(id));
			return;
		}
		else if (!pkg->steQualified) {
			XNotifier::warn(QString("包 [%1] 灭菌不合格，不能对其发放").arg(id));
			return;
		}
		QList<QStandardItem *> rowItems;
		rowItems << new QStandardItem(id);
		rowItems << new QStandardItem(pkg->name);
		rowItems << new QStandardItem(pkg->packName);
		rowItems << new QStandardItem(pkg->deparment);
		rowItems << new QStandardItem(pkg->expireDate);
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
	Package::fetchOnce(id, [this, id](Package *pkg) {
		if (pkg->state == itrac::Recycled) {
			XNotifier::warn("该包已回收，请勿重复操作");
			return;
		}

		QList<QStandardItem *> rowItems;
		rowItems.append(new QStandardItem(id));
		rowItems.append(new QStandardItem(pkg->name));
		rowItems.append(new QStandardItem(pkg->packName));
		rowItems.append(new QStandardItem(pkg->usedBy));
		rowItems.append(new QStandardItem(pkg->expireDate));
		rowItems.append(new QStandardItem("-"));
		_model->appendRow(rowItems);
	});
}

void OrRecyclePackageView::updatePlate(const QString &plateId)
{
	Plate::fetchOnce(plateId, [this](Plate* plate) {
		if (!plate->err.isEmpty()) {
			return;
		}

		if (!plate->idle) {
			return;
		}

		for (int i = 0; i != _model->rowCount(); ++i) {
			QStandardItem *item = _model->item(i, OrRecyclePackageView::VPlate);
			item->setText(plate->name);
			item->setData(plate->id);
		}
	});
}

int OrRecyclePackageView::plate() const
{
	return _model->item(0, OrRecyclePackageView::VPlate)->data().toInt();
}
