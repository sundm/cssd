#include "clinic_dispatch.h"
#include "xnotifier.h"
#include "core/application.h"
#include "barcode.h"
#include "core/net/url.h"
#include "dialog/operatorchooser.h"
#include "dialog/clinicerchooser.h"
#include "ui/views.h"
#include "ui/buttons.h"
#include "ui/composite/titles.h"
#include "ui/composite/waitingspinner.h"
#include "dialog/regexpinputdialog.h"
#include "widget/overlays/tips.h"
#include <xernel/xtimescope.h>
#include <QtWidgets/QtWidgets>

ClinicDispatchPanel::ClinicDispatchPanel(QWidget *parent)
	: CssdOverlayPanel(parent)
	, _view(new TableView)
	, _detailView(new TableView)
	, _model(new QStandardItemModel(0, 4, _view))
	, _detailModel(new QStandardItemModel(0, 2, _view))
	, _title(new Composite::Title("订单详情", false))
	, _scanView(new TableView)
	, _scanModel(new QStandardItemModel(0, 2, _scanView))
	, _scan_title(new Composite::Title("确认包列表 ", false))
	, _pktList(new QStringList())
	, _codeList(new QStringList())
	, _detailMap(new QMap<QString, int>())
	, _scanMap(new QMap<QString, int>())
	, _waiter(new WaitingSpinner(this))
{
	// setup package view info
	_model->setHeaderData(0, Qt::Horizontal, "订单号");
	_model->setHeaderData(1, Qt::Horizontal, "申请科室");
	_model->setHeaderData(2, Qt::Horizontal, "申请人");
	_model->setHeaderData(3, Qt::Horizontal, "申请时间");
	
	_view->setModel(_model);
	_view->setSelectionMode(QAbstractItemView::SingleSelection);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	connect(_view, SIGNAL(clicked(const QModelIndex &)), this, SLOT(showDetail(const QModelIndex &)));

	// setup package view info
	_detailModel->setHeaderData(0, Qt::Horizontal, "物品");
	_detailModel->setHeaderData(1, Qt::Horizontal, "数量");
	_detailView->setModel(_detailModel);
	_detailView->setSelectionMode(QAbstractItemView::SingleSelection);
	_detailView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// setup package view info
	_scanModel->setHeaderData(0, Qt::Horizontal, "条码");
	_scanModel->setHeaderData(1, Qt::Horizontal, "物品");
	_scanView->setModel(_scanModel);
	_scanView->setSelectionMode(QAbstractItemView::SingleSelection);
	_scanView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "手工添加");
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *minusButton = new Ui::IconButton(":/res/delete-24.png", "删除选中");
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	hLayout->addStretch();

	// detail panel
	QWidget *detailPanel = new QWidget(this);
	QVBoxLayout *vlayout = new QVBoxLayout(detailPanel);
	vlayout->setContentsMargins(0, 0, 0, 0);
	vlayout->setSpacing(0);
	vlayout->addWidget(_title);
	vlayout->addWidget(_detailView);
	vlayout->addLayout(hLayout);
	vlayout->addWidget(_scanView);


	// tip
	const QString text = "1 选择订单\n2 扫描或输入物品包条码\n3 确认发放\n4 扫描或输入接受人员工牌"
		"\n\n注意：\n*实际发放的物品数量应与订单数量一致";
	Tip *tip = new Tip(text);
	_commitButton = new Ui::PrimaryButton("确定发放");
	_commitButton->setEnabled(false);
	tip->addQr();
	tip->addButton(_commitButton);
	connect(_commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(_view);
	layout->addWidget(detailPanel);
	layout->addWidget(tip);
	layout->setStretch(0, 1);
	layout->setStretch(1, 2);

	QTimer::singleShot(0, this, SLOT(loadOrders()));
}

void ClinicDispatchPanel::addEntry() {
	bool ok;
	QRegExp regExp("\\d{10,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入包条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void ClinicDispatchPanel::removeEntry() {
	QItemSelectionModel *selModel = _scanView->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	if (countRow) _commitButton->setEnabled(false);

	for (int i = countRow; i > 0; i--) {
		_scanView->model()->removeRow(indexes.at(i - 1).row());
	}
		
}

void ClinicDispatchPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Package) {
		//todo
		if (!hasPackage(code)) {
			addPackage(code);
		}
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		commit();
	}
}

bool ClinicDispatchPanel::hasPackage(const QString& id) {
	QModelIndexList matches = _scanModel->match(_model->index(0, 0), Qt::UserRole + 1, id, 1);
	if (matches.isEmpty()) return false;
	else {
		return true;
	}
	
}

void ClinicDispatchPanel::addPackage(const QString& id) {
	QByteArray data("{\"package_id\":");
	data.append(id).append('}');

	post(url(PATH_PKG_INFO), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			return;
		}

		QString  pktId = QString::number(resp.getAsInt("package_type_id"));

		if (!_pktList->contains(pktId))
		{
			XNotifier::warn(QString("该条码%1所对应的包，不在当前订单中。").arg(id), -1);
			return;
		}
		
		if (_codeList->contains(id))
		{
			XNotifier::warn(QString("该条码包已录入，请勿重复扫描: ").append(id), -1);
			return;
		}

		if (resp.getAsString("state") != "ST") {
			XNotifier::warn(QString("包 [%1] 尚未完成灭菌审核，或者已发放").arg(id));
			return;
		}

		if (!resp.getAsBool("sterilize_qualified")) {
			XNotifier::warn(QString("该条码包灭菌未合格"), -1);
			return;
		}

		if (resp.getAsBool("expired")) {
			XNotifier::warn(QString("该条码包已过期"), -1);
			return;
		}

		int num = _scanMap->value(pktId);
		if (num < _detailMap->value(pktId))
		{
			num++;
			_scanMap->insert(pktId, num);
		}
		else
		{
			XNotifier::warn(QString("该类型包数量已满：共%1个").arg(num), -1);
			return;
		}

		QList<QVariant> pkgTypes = resp.getAsList("package_types");
		QList<QStandardItem *> rowItems;
		QStandardItem *item_code = new QStandardItem();
		item_code->setTextAlignment(Qt::AlignCenter);
		item_code->setText(id);
		rowItems.append(item_code);
		_codeList->append(id);

		QStandardItem *item_pkg_name = new QStandardItem();
		item_pkg_name->setTextAlignment(Qt::AlignCenter);
		item_pkg_name->setText(resp.getAsString("package_type_name"));
		item_pkg_name->setData(resp.getAsInt("package_type_id"));
		rowItems.append(item_pkg_name);

		_scanModel->appendRow(rowItems);

		_commitButton->setEnabled(checkNumber());
	});
}

bool ClinicDispatchPanel::checkNumber() {
	bool bIsMatch = true;
	QMapIterator<QString, int> i(*_detailMap);
	while (i.hasNext()) {
		if (_scanMap->value(i.next().key()) != i.value())
			bIsMatch = false;
	}

	return bIsMatch;
}

void ClinicDispatchPanel::reset() {
	_detailView->clear();
	_scanView->clear();
	_title->setTitle("订单详情");
	loadOrders();
}

void ClinicDispatchPanel::commit() {
	QModelIndexList indices = _view->selectedRows();
	if (indices.isEmpty()) return;
	if (indices.size() > 1) return;
	
	QModelIndex index = indices.first();

	int order = _model->data(_model->index(index.row(), 0)).toInt();
	int deptId = _model->data(_model->index(index.row(), 1), Qt::UserRole + 1).toInt();

	int recvId = ClinicerChooser::get(this, this);
	if (0 == recvId) return;

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	QVariantList pkgList;
	for (int i = 0; i < _scanModel->rowCount(); i++) {
		QString pkgIdStr = _scanModel->item(i, 0)->text();
		pkgList.append(pkgIdStr);
	}

	QVariantMap vmap;
	vmap.insert("package_ids", pkgList); // TODO
	vmap.insert("department_id", deptId);
	vmap.insert("operator_id", opId);
	vmap.insert("order_id", order);
	vmap.insert("r_operator_id", recvId);

	post(url(PATH_ISSUE_ADD), vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success())
			XNotifier::warn(QString("发放登记错误: ").append(resp.errorString()));
		else {
			XNotifier::warn("已完成发放登记");
			reset();
		}
	});
}

void ClinicDispatchPanel::loadOrders() {
	QVariantMap vmap;
	XDateScope timeScope(XPeriod::ThisYear);
	vmap.insert("state", "1");
	vmap.insert("start_time", timeScope.from);
	vmap.insert("end_time", timeScope.to);

	_waiter->start();
	post(url(PATH_ORDER_SEARCH), vmap, [this](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法查询: ").append(resp.errorString()));
			return;
		}

		_view->clear();

		QList<QVariant> orders = resp.getAsList("orders_info");
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();
			
			QList<QStandardItem *> rowItems;
			QStandardItem *item_orderId = new QStandardItem();
			item_orderId->setTextAlignment(Qt::AlignCenter);
			item_orderId->setText(map["order_id"].toString());
			rowItems.append(item_orderId);

			QStandardItem *item_dept = new QStandardItem();
			item_dept->setTextAlignment(Qt::AlignCenter);
			item_dept->setText(map["dept_name"].toString());
			item_dept->setData(map["dept_id"]);
			rowItems.append(item_dept);

			QStandardItem *item_s_name = new QStandardItem();
			item_s_name->setTextAlignment(Qt::AlignCenter);
			item_s_name->setText(map["s_name"].toString());
			item_s_name->setData(map["s_operator_id"]);
			rowItems.append(item_s_name);

			QStandardItem *item_s_date = new QStandardItem();
			item_s_date->setTextAlignment(Qt::AlignCenter);
			item_s_date->setText(map["s_date"].toString());
			rowItems.append(item_s_date);

			_model->appendRow(rowItems);
		}
	});
}

void ClinicDispatchPanel::showDetail(const QModelIndex &index) {
	int row = index.row();
	int order = _model->data(_model->index(row, 0)).toInt();
	QString deptName = _model->data(_model->index(row, 1)).toString();
	_title->setTitle(QString("%1(%2)").arg(deptName).arg(order));

	_detailView->clear();
	_scanView->clear();
	_codeList->clear();
	_pktList->clear();
	_detailMap->clear();
	_scanMap->clear();

	QByteArray data("{\"order_id\":");
	data.append(QString::number(order)).append('}');
	_waiter->start();
	post(url(PATH_ORDER_PKGINFO), data, [this](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			return;
		}

		QList<QVariant> orders = resp.getAsList("orders_pkg");
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();

			QList<QStandardItem *> rowItems;
			QStandardItem *item_pkg_name = new QStandardItem();
			item_pkg_name->setTextAlignment(Qt::AlignCenter);
			item_pkg_name->setText(map["package_type_name"].toString());
			item_pkg_name->setData(map["package_type_id"].toString());
			rowItems.append(item_pkg_name);
			_pktList->append(map["package_type_id"].toString());

			QStandardItem *item_num = new QStandardItem();
			item_num->setTextAlignment(Qt::AlignCenter);
			item_num->setText(map["num"].toString());
			rowItems.append(item_num);

			_detailMap->insert(map["package_type_id"].toString(), map["num"].toInt());

			_detailModel->appendRow(rowItems);
		}
	});
}
