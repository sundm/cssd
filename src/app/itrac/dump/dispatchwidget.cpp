#include "dispatchwidget.h"
#include "deptpicker.h"
#include "jsonhttpclient.h"
#include "jsonhttpresponse.h"
#include "urls.h"
#include "Barcode.h"
#include "xnotifier.h"
#include <QStandardItemModel>
#include <QNetworkReply>
#include <qscanner/qscanner.h>

DispatchWidget::DispatchWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	initPackageView();

	connect(deptSearchButton, &QAbstractButton::clicked, this, &DispatchWidget::pickDept);
	connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
	connect(dispatchButton, SIGNAL(clicked()), this, SLOT(doDispatch()));

	connect(backButton, SIGNAL(clicked()), parent, SLOT(goBack()));
	QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
}

DispatchWidget::~DispatchWidget()
{
}

void DispatchWidget::initPackageView()
{
	_pkgModel = new QStandardItemModel(0, 4, pkgTableView);
	_pkgModel->setHeaderData(0, Qt::Horizontal, "条码");
	_pkgModel->setHeaderData(1, Qt::Horizontal, "包类型");
	_pkgModel->setHeaderData(2, Qt::Horizontal, "归属科室");
	_pkgModel->setHeaderData(3, Qt::Horizontal, "失效期");
	pkgTableView->setModel(_pkgModel);

	pkgTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	pkgTableView->setSelectionMode(QAbstractItemView::SingleSelection);
	pkgTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = pkgTableView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 200);
	header->resizeSection(1, 200);
	header->resizeSection(2, 200);
}

void DispatchWidget::barcodeScanned(const QString & bc)
{
	Barcode barcode(bc);
	int codeType = barcode.type();
	if (Barcode::Department == codeType)
		updateDept(bc);
	else if (Barcode::Package == codeType)
		updatePackageView(bc);
	else if (Barcode::User == codeType)
		updateUser(bc);
}

void DispatchWidget::pickDept()
{
	DeptPicker deptPicker;
	/*connect(deptPicker, &DeptPicker::deptPicked,
		this, QOverload<int, const QString&>::of(&DispatchWidget::updateDept));*/
	connect(&deptPicker, &DeptPicker::deptPicked,
		this, QOverload<int, const QString&>::of(&DispatchWidget::updateDept));
	deptPicker.exec();
}

void DispatchWidget::updateDept(int id, const QString &name) {
	deptNameLabel->setText(name);
	deptIdEdit->setText(QString::number(id));
}

void DispatchWidget::updateDept(const QString &deptId)
{
	QByteArray data("{\"department_id\":");
	data.append(deptId).append('}');

	JsonHttpClient().post(url(PATH_PLATE_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取编号[%1]的科室信息").arg(deptId), -1);
			return;
		}

		QList<QVariant> depts = resp.getAsList("plates");
		if (depts.isEmpty()) {
			XNotifier::warn(this, QString("编号[%1]的科室不在系统资产目录中").arg(deptId), -1);
			return;
		}
		QVariantMap map = depts[0].toMap();
		deptNameLabel->setText(map["department_name"].toString());
		deptIdEdit->setText(deptId);
	});
}

void DispatchWidget::updateUser(const QString &userId)
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

		userNameLabel->setText(map["operator_name"].toString());
		userIdEdit->setText(userId);
	});
}

void DispatchWidget::updatePackageView(const QString &pkgId)
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
		bool sterileQualified = resp.getAsBool("sterilize_qualified");
		if (state != "ST") {
			XNotifier::warn(this, QString("包 [%1] 尚未完成灭菌审核，或者已发放").arg(pkgId), -1);
			return;
		}
		else if (!sterileQualified) {
			XNotifier::warn(this, QString("包 [%1] 灭菌不合格，不能对其发放").arg(pkgId), -1);
			return;
		}
		QString depName = resp.getAsString("department_name");
		QString pkgTypeName = resp.getAsString("package_type_name");
		QList<QStandardItem *> rowItems;
		rowItems.append(new QStandardItem(pkgId));
		rowItems.append(new QStandardItem(pkgTypeName));
		rowItems.append(new QStandardItem(depName));
		rowItems.append(new QStandardItem(resp.getAsString("valid_date")));
		_pkgModel->appendRow(rowItems);
	});
}

void DispatchWidget::doDispatch()
{
	if (0 == _pkgModel->rowCount()
		|| deptIdEdit->text().isEmpty()
		|| userIdEdit->text().isEmpty()) {
		XNotifier::warn(this, "发放信息不完整，请扫描或手工输入所需要的发放信息", -1);
		return;
	}

	QVariantMap vmap;
	vmap.insert("department_id", deptIdEdit->text().toInt());
	vmap.insert("operator_id", userIdEdit->text().toInt());

	QVariantList pkgList;
	for (int i = 0; i != _pkgModel->rowCount(); i++) {
		QString pkgId = _pkgModel->item(i, 0)->text();
		pkgList.push_back(pkgId);
	}
	vmap.insert("package_ids", pkgList);

	JsonHttpClient().post(url(PATH_ISSUE_ADD), vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("提交发放失败: ").append(resp.errorString()), -1);
		}
		else {
			XNotifier::warn(this, "发放成功!", -1);
			reset();
		}
	});
}

void DispatchWidget::reset()
{
	_pkgModel->removeRows(0, _pkgModel->rowCount());
	deptNameLabel->clear();
	deptIdEdit->clear();
}
