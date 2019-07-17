#include "clinic.h"
#include "xnotifier.h"
#include "core/barcode.h"
#include "core/application.h"
#include "core/net/url.h"
#include "core/assets.h"
#include "dialog/operatorchooser.h"
#include "ui/views.h"
#include "ui/buttons.h"
#include "ui/composite/titles.h"
#include "ui/composite/waitingspinner.h"
#include "dialog/regexpinputdialog.h"
#include "widget/overlays/tips.h"
#include <xernel/xtimescope.h>
#include <QtWidgets/QtWidgets>

ClinicPanel::ClinicPanel(QWidget *parent)
	: CssdOverlayPanel(parent)
	, _view(new TableView)
	, _detailView(new TableView)
	, _model(new QStandardItemModel(0, 3, _view))
	, _detailModel(new QStandardItemModel(0, 3, _view))
	, _title(new Composite::Title("订单详情", false))
	, _waiter(new WaitingSpinner(this))
{
	// setup package view info
	_model->setHeaderData(0, Qt::Horizontal, "订单号");
	_model->setHeaderData(1, Qt::Horizontal, "科室");
	_model->setHeaderData(2, Qt::Horizontal, "订单生成时间");
	//_model->setHeaderData(3, Qt::Horizontal, "操作");
	_view->setModel(_model);
	_view->setSelectionMode(QAbstractItemView::SingleSelection);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(_view, SIGNAL(clicked(const QModelIndex &)), this, SLOT(showDetail(const QModelIndex &)));

	// setup package view info
	_detailModel->setHeaderData(0, Qt::Horizontal, "物品");
	_detailModel->setHeaderData(1, Qt::Horizontal, "数量");
	_detailModel->setHeaderData(2, Qt::Horizontal, "装篮");
	_detailView->setModel(_detailModel);
	_detailView->setSelectionMode(QAbstractItemView::SingleSelection);
	_detailView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	Ui::IconButton *addPlateButton = new Ui::IconButton(":/res/fill-plate-24.png");
	addPlateButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	connect(addPlateButton, SIGNAL(clicked()), this, SLOT(addPlate()));

	// detail panel
	QWidget *detailPanel = new QWidget(this);
	QVBoxLayout *vlayout = new QVBoxLayout(detailPanel);
	vlayout->setContentsMargins(0, 0, 0, 0);
	vlayout->setSpacing(0);
	vlayout->addWidget(_title);
	vlayout->addWidget(addPlateButton);
	vlayout->addWidget(_detailView);

	// tip
	const QString text = "1 选择订单\n2 扫描或输入托盘编号\n3 确定回收"
		"\n\n注意：\n*实际回收的物品数量应与订单数量一致\n*超过1周未处理的订单系统自动作废";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("确定回收");
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(_view);
	layout->addWidget(detailPanel);
	layout->addWidget(tip);
	layout->setStretch(0, 3);
	layout->setStretch(1, 2);

	QTimer::singleShot(0, this, SLOT(loadOrders()));
}

void ClinicPanel::addPlate() {
	bool ok;
	QRegExp regExp("\\d{8,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入篮筐条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void ClinicPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Plate) {
		updatePlate(code);
	}
}

void ClinicPanel::updatePlate(const QString &plateId) {
	QByteArray data("{\"plate_id\":");
	data.append(plateId).append('}');
	post(url(PATH_PLATE_SEARCH), data, [plateId, this](QNetworkReply *reply) {
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

		for (int i = 0; i != _detailModel->rowCount(); ++i) {
			QStandardItem *item = _detailModel->item(i, 2);
			item->setText(map["plate_name"].toString());
			item->setData(plateId.toInt());
		}
	});
}

void ClinicPanel::reset() {
	_detailView->clear();
	_title->setTitle("订单详情");
	loadOrders();
}

void ClinicPanel::commit() {
	QModelIndexList indices = _view->selectedRows();
	if (indices.isEmpty()) {
		XNotifier::warn("请选择需要回收的包");
		return;
	}

	QVariantList orders;
	for (auto &idx : indices) {
		orders << _model->data(_model->index(idx.row(), 0)).toInt();
	}

	int plateId = _detailModel->item(0, 2)->data().toInt();
	if (0 == plateId) {
		XNotifier::warn("请添加网篮");
		return;
	}

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	QVariantMap vmap;
	vmap.insert("plate_id", plateId); // TODO
	vmap.insert("operator_id", opId);
	vmap.insert("order_ids", orders);

	post(url(PATH_ORDER_RECYCLE), vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success())
			XNotifier::warn(QString("回收登记错误: ").append(resp.errorString()));
		else {
			XNotifier::warn("已完成回收登记");
			reset();
		}
	});
}

void ClinicPanel::loadOrders() {
	QVariantMap vmap;
	XDateScope timeScope(XPeriod::RecentWeek);
	vmap.insert("state", "0");
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
		_model->insertRows(0, orders.count());
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();
			_model->setData(_model->index(i, 0), map["order_id"]);
			_model->setData(_model->index(i, 1), map["dept_name"]);
			_model->setData(_model->index(i, 2), map["s_date"]);
			//_view->setIndexWidget(_model->index(i, 3), new QPushButton("查看详情"));
		}
	});
}

void ClinicPanel::showDetail(const QModelIndex &index) {
	int row = index.row();
	int order = _model->data(_model->index(row, 0)).toInt();
	QString deptName = _model->data(_model->index(row, 1)).toString();
	_title->setTitle(QString("%1(%2)").arg(deptName).arg(order));
	_detailView->clear();

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
		_detailModel->insertRows(0, orders.count());
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();
			_detailModel->setData(_detailModel->index(i, 0), map["package_type_name"]);
			_detailModel->setData(_detailModel->index(i, 1), map["num"]);
			_detailModel->setData(_detailModel->index(i, 2), QString("请扫描篮筐条码"));
		}
	});
}
