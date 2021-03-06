#include "plateview.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include "util/printermanager.h"
#include <printer/labelprinter.h>
#include <QStandardItemModel>
#include <QHeaderView>

PlateView::PlateView(QWidget *parent)
	: QTreeView(parent), _model(new QStandardItemModel(0, 2, this))
{
	_model->setHeaderData(0, Qt::Horizontal, "托盘/物品类型");
	_model->setHeaderData(1, Qt::Horizontal, "物品数量");
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = this->header();
	header->setSectionResizeMode(QHeaderView::Stretch);
}

bool PlateView::hasPlate(int id) const
{
	QModelIndexList matches = _model->match(_model->index(0, 0), Qt::UserRole + 1, id, 1);
	return !matches.isEmpty();
}

void PlateView::addPlate(int id) {
	QByteArray data("{\"plate_id\":");
	data.append(QString::number(id)).append('}');

	Url::post(Url::PATH_PKG_IN_PLATE, data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//XNotifier::warn(this, QString("无法获取清洗网篮数据: ").append(resp.errorString()), -1);
			return;
		}

		QList<QVariant> pkgTypes = resp.getAsList("package_types");
		if (pkgTypes.isEmpty()) {
			return;
		}

		QStandardItem *plateItem = new QStandardItem(QString("托盘 %1").arg(id));
		plateItem->setData(id);

		for(auto &pkgType: pkgTypes) {
			QVariantMap map = pkgType.toMap();
			QStandardItem *nameItem = new QStandardItem(map["package_type_name"].toString());
			nameItem->setData(map["package_type_id"]);
			QStandardItem *numberItem = new QStandardItem(map["package_type_num"].toString());
			QList<QStandardItem *> rowItems;
			rowItems << nameItem << numberItem;
			plateItem->appendRow(rowItems);
		}
		_model->appendRow(plateItem);
		setExpanded(plateItem->index(), true);
	});
}

QVariantList PlateView::plates() const {
	QVariantList vec;
	vec.reserve(2);
	for (int i = 0; i != _model->rowCount(); i++) {
		int plateId = _model->data(_model->index(i, 0), Qt::UserRole + 1).toInt();
		vec.push_back(plateId);
	}
	return vec;
}

void PlateView::clear() {
	_model->removeRows(0, _model->rowCount());
}

PackPlateView::PackPlateView(QWidget *parent)
	: QTreeView(parent), _model(new QStandardItemModel(0, ExpireDate + 1, this))
{
	_model->setHeaderData(Package, Qt::Horizontal, "托盘/待配物品");
	_model->setHeaderData(PackType, Qt::Horizontal, "打包方式");
	_model->setHeaderData(Barcode, Qt::Horizontal, "条码");
	_model->setHeaderData(SterileDate, Qt::Horizontal, "灭菌日期");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = this->header();
	header->setSectionResizeMode(QHeaderView::Stretch);
}

void PackPlateView::addPlate(int id)
{
	QByteArray data("{\"plate_id\":");
	data.append(QString::number(id)).append('}');

	Url::post(Url::PATH_PKG_IN_PLATE, data, [this, id](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取清洗网篮数据: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> pkgTypes = resp.getAsList("package_types");
		if (pkgTypes.isEmpty()) {
			return;
		}

		QStandardItem *plateItem = new QStandardItem(QString("托盘 %1").arg(id));
		plateItem->setData(id);

		for(auto &pkgType: pkgTypes) {
			QVariantMap map = pkgType.toMap();
			QString pkgName = map["package_type_name"].toString();
			QString packName = map["pack_type_name"].toString();
			QVariant pkgId = map["package_type_id"];
			QVariant packId = map["pack_type_id"];
			int num = map["package_type_num"].toInt();
			while (num--) {
				QStandardItem *nameItem = new QStandardItem(pkgName);
				nameItem->setData(pkgId);
				QStandardItem *packItem = new QStandardItem(packName);
				packItem->setData(packId);
				QList<QStandardItem *> row;
				row << nameItem << packItem << new QStandardItem;
				plateItem->appendRow(row);
			}
		}
		_model->appendRow(plateItem);
		setExpanded(plateItem->index(), true);
	});
}

QVariantList PackPlateView::plates() const {
	QVariantList vec;
	for (int i = 0; i != _model->rowCount(); i++) {
		int plateId = _model->data(_model->index(i, 0), Qt::UserRole + 1).toInt();
		vec.push_back(plateId);
	}
	return vec;
}

void PackPlateView::doPack(int opId, int checkId) {
	// check printer
	LabelPrinter *printer = PrinterManager::currentPrinter();
	if (0 != printer->open("ZDesigner GT800 (ZPL)")) {
		XNotifier::warn("打印机未就绪");
		return;
	}

	QVariantList pkgList;
	QList<int> rows = getSelectedPackages(pkgList);

	QVariantMap vmap;
	vmap.insert("plate_id", plates().at(0));
	vmap.insert("packages", pkgList);
	vmap.insert("operator_id", opId);
	vmap.insert("check_operator_id", checkId);

	Url::post(Url::PATH_PACK_ADD, vmap, [this, rows, printer](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			printer->close();
			XNotifier::warn(QString("配包失败: ").append(resp.errorString()));
			return;
		}

		QVariantList pkgList = resp.getAsList("pack_info");
		if (pkgList.count() != rows.count()) {
			printer->close();
			XNotifier::warn("系统内部错误，无法生成对应数量的条码");
			return;
		}

		QStandardItem *plateItem = _model->item(0, 0);
		for (int i = 0; i != rows.count(); ++i) {
			const QVariantMap &pkg = pkgList.at(i).toMap();
			plateItem->setChild(i, Barcode, new QStandardItem(pkg["package_id"].toString()));
			plateItem->setChild(i, SterileDate, new QStandardItem(pkg["sterilize_date"].toString()));
			plateItem->setChild(i, ExpireDate, new QStandardItem(pkg["expire_date"].toString()));
		}

		// generate labels for print
		PackageLabel label;
		label.operatorName = resp.getAsString("pack_operator_name");
		label.assessorName = resp.getAsString("check_operator_name");
		for(auto &pkg: pkgList) {
			QVariantMap map = pkg.toMap();
			label.packageId = map["package_id"].toString();
			label.packageName = map["package_name"].toString();
			label.packageFrom = map["department_name"].toString();
			label.packageType = map["pack_type_name"].toString();
			label.disinDate = map["sterilize_date"].toString();
			label.expiryDate = map["expire_date"].toString();
			label.count = map["instrument_num"].toInt();
			printer->printPackageLabel(label);
		}
	});
}

/* return list of selected row number */
QList<int> PackPlateView::getSelectedPackages(QVariantList &list) const {
	QList<int> rows;

	QStandardItem *plateItem = _model->item(0, 0);
	for (int i = 0; i != plateItem->rowCount(); i++) {
		QVariantMap packageType;
		packageType.insert("package_type_id", plateItem->child(i, 0)->data());
		packageType.insert("pack_type_id", plateItem->child(i, 1)->data());
		list << packageType;
		rows << i;
	}

	return rows;
}
