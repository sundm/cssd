#include "plateview.h"
#include "core/net/url.h"
#include "inliner.h"
#include "xnotifier.h"
#include "util/printermanager.h"
#include "rdao/dao/PackageDao.h"
#include "rdao/entity/operator.h"
#include "rdao/entity/package.h"
#include <printer/labelprinter.h>
#include <QStandardItemModel>
#include <QHeaderView>

PlateView::PlateView(QWidget *parent)
	: TableView(parent)
	, _model(new QStandardItemModel(0, 2, this))
{
	_model->setHeaderData(0, Qt::Horizontal, "器械包UDID");
	_model->setHeaderData(1, Qt::Horizontal, "包内器械数量");
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	_pkgList.clear();
}

void PlateView::addPackage(const QString& udi) {
	for each (Package pkg in _pkgList)
	{
		if (udi.compare(pkg.udi) == 0)
		{
			XNotifier::warn(QString("该器械包已经添加:").append(udi), -1);
			return;
		}
	}

	PackageDao dao;
	Package pkg;
	result_t resp = dao.getPackage(udi, &pkg, true);
	if (resp.isOk())
	{
		_pkgList.append(pkg);

		QList<QStandardItem *> rowItems;
		QStandardItem *plateItem = new QStandardItem(udi);
		plateItem->setData(udi);
		QStandardItem *numberItem = new QStandardItem(QString::number(pkg.instruments.size()));
		rowItems << plateItem << numberItem;
		_model->appendRow(rowItems);		
	}
	else
	{
		XNotifier::warn(QString("无法获取器械包数据:").append(resp.msg()), -1);
	}
	/*
	QByteArray data("{\"plate_id\":");
	data.append(QString::number(id)).append('}');

	_http.post(url(PATH_PKG_IN_PLATE), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取清洗网篮数据: ").append(resp.errorString()), -1);
			return;
		}

		QList<QVariant> pkgTypes = resp.getAsList("package_types");
		if (pkgTypes.isEmpty()) {
			XNotifier::warn(QString("该清洗网篮没有包信息"), -1);
			return;
		}

		clear();

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
	});
	*/
}

const QList<Package> &PlateView::packages() const {
	return _pkgList;
}

void PlateView::clear() {
	_model->removeRows(0, _model->rowCount());
	_pkgList.clear();
}

PackPlateView::PackPlateView(QWidget *parent)
	: QTreeView(parent), _model(new QStandardItemModel(0, ExpireDate + 1, this))
{
	_model->setHeaderData(Package, Qt::Horizontal, "托盘/待配物品");
	_model->setHeaderData(PackType, Qt::Horizontal, "打包方式");
	_model->setHeaderData(Barcode, Qt::Horizontal, "条码");
	_model->setHeaderData(CardId, Qt::Horizontal, "包内器械清单号");
	_model->setHeaderData(SterileDate, Qt::Horizontal, "灭菌日期");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = this->header();
	header->setSectionResizeMode(QHeaderView::Stretch);

}

void PackPlateView::clear()
{
	_model->removeRows(0, _model->rowCount());
}

void PackPlateView::addPlate(int id)
{
	QByteArray data("{\"plate_id\":");
	data.append(QString::number(id)).append('}');

	_http.post(url(PATH_PKG_IN_PLATE), data, [this, id](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取清洗网篮数据: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> pkgTypes = resp.getAsList("package_types");
		if (pkgTypes.isEmpty()) {
			XNotifier::warn(QString("该网篮为空！"));
			return;
		}

		_model->removeRows(0, _model->rowCount());

		QStandardItem *plateItem = new QStandardItem(QString("托盘 %1").arg(id));
		plateItem->setData(id);

		for(auto &pkgType: pkgTypes) {
			QVariantMap map = pkgType.toMap();
			QString pkgName = map["package_type_name"].toString();
			QString packName = map["pack_type_name"].toString();
			QString pkgId = map["pkg_id"].toString();
			QVariant pkgTypeId = map["package_type_id"];
			QVariant packId = map["pack_type_id"];
			QString cardId = map["package_card"].toString();
			int steType = map["card_record"].toInt();
			int pkgNum = map["package_type_num"].toInt();
			for (int i = 0; i < pkgNum; i++)
			{
				QStandardItem *nameItem = new QStandardItem(pkgName);
				nameItem->setData(brushForSteType(steType), Qt::BackgroundRole);
				nameItem->setData(steType, Qt::UserRole + 2);
				nameItem->setData(pkgTypeId);
				QStandardItem *packItem = new QStandardItem(packName);
				packItem->setData(packId);
				QStandardItem *pkgIdItem = new QStandardItem(pkgId);
				pkgIdItem->setData(pkgId);
				QStandardItem *cardIdItem = new QStandardItem(cardId);
				cardIdItem->setData(cardId);
				QList<QStandardItem *> row;
				row << nameItem << packItem << pkgIdItem << cardIdItem << new QStandardItem;
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
	if (0 != printer->open(LABEL_PRINTER)) {
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

	_http.post(url(PATH_PACK_ADD), vmap, [this, rows, printer](QNetworkReply *reply) {
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

		ClinicLabel cLabel;
		PackageLabel pLabel;
			
			
		for (auto &pkg : pkgList) {
			QVariantMap map = pkg.toMap();
			if (map["package_category"].toInt() == 1)
			{
				cLabel.operatorName = resp.getAsString("pack_operator_name");
				cLabel.assessorName = resp.getAsString("check_operator_name");
				cLabel.packageId = map["package_id"].toString();
				cLabel.packageName = map["package_name"].toString();
				cLabel.packageFrom = map["department_name"].toString();
				cLabel.disinDate = map["sterilize_date"].toString();
				cLabel.expiryDate = map["expire_date"].toString();
				printer->printClinicLabel(cLabel);
			}
			else
			{
				pLabel.operatorName = resp.getAsString("pack_operator_name");
				pLabel.assessorName = resp.getAsString("check_operator_name");
				pLabel.packageId = map["package_id"].toString();
				pLabel.packageName = map["package_name"].toString();
				pLabel.packageFrom = map["department_name"].toString();
				pLabel.packageType = map["pack_type_name"].toString();
				pLabel.disinDate = map["sterilize_date"].toString().left(10);
				pLabel.expiryDate = map["expire_date"].toString();
				pLabel.count = map["instrument_num"].toInt();
				printer->printPackageLabel(pLabel);
			}
				
		}
		
		printer->close();
		clear();
	});
}

/* return list of selected row number */
QList<int> PackPlateView::getSelectedPackages(QVariantList &list) const {
	QList<int> rows;

	QStandardItem *plateItem = _model->item(0, 0);
	for (int i = 0; i != plateItem->rowCount(); i++) {
		QVariantMap packageType;
		int state = 0;
		state = plateItem->child(i, 0)->data(Qt::UserRole + 2).toInt();
		QString name = plateItem->child(i, 0)->text();
		if (state < 2)
		{
			packageType.insert("package_type_id", plateItem->child(i, 0)->data());
			packageType.insert("pack_type_id", plateItem->child(i, 1)->data());
			int card_id = 0;
			card_id = plateItem->child(i, 3)->data().toInt();
			packageType.insert("card_id", card_id);
			list << packageType;
			rows << i;
		}
		else {
			XNotifier::warn(QString("%1标记异常，无法进行配包，已跳过").arg(name));
		}
	}

	return rows;
}
