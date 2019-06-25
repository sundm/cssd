#include <qdebug>
#include <QStandardItemModel>
#include <QNetworkReply>
#include <qscanner/qscanner.h>
#include "controls/regexpinputdialog.h"

#include "RecycleWidget.h"
#include "SelectPkgDialog.h"
#include "jsonhttpclient.h"
#include "jsonhttpresponse.h"
#include "operatorchooser.h"
#include "urls.h"
#include "Barcode.h"
#include "xnotifier.h"

RecycleWidget::RecycleWidget(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);

	//backButton->setText(QChar(0xf053)); // fa-chevron-left

	initPackageTableView();

	connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
	connect(bcInputButton, SIGNAL(clicked()), this, SLOT(inputBarcode()));
	//connect(recycleButton, SIGNAL(clicked()), this, SLOT(doRecycle()));

	QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
}

RecycleWidget::~RecycleWidget()
{
}

void RecycleWidget::barcodeScanned(const QString & bc)
{
	Barcode barcode(bc);
	int codeType = barcode.type();
	if (Barcode::Plate == codeType)
		updatePlate(bc);
	else if (Barcode::Package == codeType)
		updatePackageView(bc);
	else if (Barcode::User == codeType)
		updateUser(bc);
}

void RecycleWidget::initPackageTableView()
{
	_pkgModel = new QStandardItemModel(0, 3, pkgTableView);
	_pkgModel->setHeaderData(0, Qt::Horizontal, "条码");
	_pkgModel->setHeaderData(1, Qt::Horizontal, "来源科室");
	_pkgModel->setHeaderData(2, Qt::Horizontal, "包类型");
	pkgTableView->setModel(_pkgModel);

	pkgTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	pkgTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	pkgTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = pkgTableView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 300);
	header->resizeSection(1, 260);
}

void RecycleWidget::updatePlate(const QString &plateId)
{
	QByteArray data("{\"plate_id\":");
	data.append(plateId).append('}');

	JsonHttpClient().post(url(PATH_PLATE_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取编号[%1]的网篮信息").arg(plateId), -1);
			return;
		}

		QList<QVariant> plates = resp.getAsList("plates");
		if (plates.isEmpty()) {
			XNotifier::warn(this, QString("编号[%1]的网篮不在系统资产目录中").arg(plateId), -1);
			return;
		}

		QVariantMap map = plates[0].toMap();
		bool isFinished = "1" == map["is_finished"].toString();
		if (!isFinished) {
			XNotifier::warn(this, QString("编号[%1]的网篮正在使用中，请选择其他网篮").arg(plateId), -1);
			return;
		}
	});
}

void RecycleWidget::updatePackageView(const QString &pkgId)
{
	QList<QStandardItem *> items = _pkgModel->findItems(pkgId, Qt::MatchExactly, 0);
	if (0 != items.size())
		return;

	QString data = QString("{\"package_id\":\"%1\"}").arg(pkgId);
	JsonHttpClient().post(url(PATH_PKG_INFO), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取包 [%1] 的信息").arg(pkgId), -1);
			return;
		}

		QString state = resp.getAsString("state");
		if (state == "R") {
			XNotifier::warn(this, "该包已回收，请勿重复操作", -1);
			return;
		}

		QString depName = resp.getAsString("from_department_name");
		QString pkgTypeName = resp.getAsString("package_type_name");
		QList<QStandardItem *> rowItems;
		rowItems.append(new QStandardItem(pkgId));
		rowItems.append(new QStandardItem(depName));
		rowItems.append(new QStandardItem(pkgTypeName));
		_pkgModel->appendRow(rowItems);
	});
}

void RecycleWidget::updateUser(const QString &userId)
{
	QByteArray data("{\"operator_id\":");
	data.append(userId).append('}');

	JsonHttpClient().post(url(PATH_USER_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取编号[%1]的用户信息").arg(userId), -1);
			return;
		}

		QList<QVariant> users = resp.getAsList("user_list");
		if (users.isEmpty()) {
			XNotifier::warn(this, QString("编号[%1]的用户不在系统资产目录中").arg(userId), -1);
			return;
		}

		QVariantMap map = users[0].toMap();
		bool isForbidden = "1" == map["is_forbidden"].toString();
		if (isForbidden) {
			XNotifier::warn(this, QString("编号[%1]的用户已被禁用，请联系管理员").arg(userId), -1);
			return;
		}

	});
}

void RecycleWidget::reset()
{
	_pkgModel->removeRows(0, _pkgModel->rowCount());
}

void RecycleWidget::inputBarcode()
{
	bool ok;
	QRegExp regExp("\\d{10,}");
	QString text = RegExpInputDialog::getText(this, "手工输入条码", "请输入包上的条码", "", regExp, &ok);
	if (ok) {
		barcodeScanned(text);
	}
}

void RecycleWidget::selectPackage()
{
	SelectPkgDialog dialog(this);
	connect(&dialog, SIGNAL(packageSelected(const QString &, int, const QString &, int)),
		this, SLOT(insertNoBarcodePackage(const QString &, int, const QString &, int)));
	if (QDialog::Accepted == dialog.exec()) {

	}
}

void RecycleWidget::doRecycle()
{
	if (0 == _pkgModel->rowCount() ) {
		XNotifier::warn(this, "回收信息不完整，请扫描或手工输入所需要的回收信息", -1);
		return;
	}
	//ConfirmDialog dialog;
	//if (QDialog::Accepted == dialog.exec()) {
	QVariantList pkgList;
	QVariantList pkgTypeList;

	for (int i = 0; i != _pkgModel->rowCount(); i++) {
		QString bc = _pkgModel->item(i, 0)->text();
		if (bc.isEmpty()) {
			QVariantMap packageType;
			packageType.insert("package_type_id", _pkgModel->item(i, 2)->data(Qt::UserRole + 1));
			packageType.insert("department_id", _pkgModel->item(i, 1)->data(Qt::UserRole + 1));
			packageType.insert("add_type", 5);
			pkgTypeList << packageType;
		}
		else {
			QVariantMap package;
			package.insert("package_id", bc);
			package.insert("recycle_reason", 1);
			pkgList << package;
		}
	}

	QVariantMap vmap;
	/*vmap.insert("plate_id", plateIdEdit->text().toInt());
	vmap.insert("operator_id", userIdEdit->text().toInt());*/
	if (!pkgList.isEmpty()) {
		vmap.insert("packages", pkgList);
	}
	if (!pkgTypeList.isEmpty()) {
		vmap.insert("package_types", pkgTypeList);
	}

	JsonHttpClient().post(url(PATH_RECYCLE_ADD), vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("回收登记失败: ").append(resp.errorString()), -1);
		}
		else {
			XNotifier::warn(this, "回收成功!", -1);
			reset();
		}
	});
}

void RecycleWidget::insertNoBarcodePackage(const QString &depName, int depId, const QString &pkgTypeName, int pkgTypeId)
{
	QList<QStandardItem *> rowItems;
	QStandardItem *depItem = new QStandardItem(depName);
	depItem->setData(depId);
	QStandardItem *pkgItem = new QStandardItem(pkgTypeName);
	pkgItem->setData(pkgTypeId);
	rowItems.append(new QStandardItem);
	rowItems.append(depItem);
	rowItems.append(pkgItem);
	_pkgModel->appendRow(rowItems);
}
