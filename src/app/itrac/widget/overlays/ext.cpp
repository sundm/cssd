#include "ext.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/net/url.h"
#include "core/inliner.h"
#include "dialog/extdialog.h"
#include "dialog/repayextdialog.h"
#include "ui/views.h"
#include "ui/buttons.h"
#include "ui/composite/titles.h"
#include "widget/overlays/tips.h"
#include <xernel/xtimescope.h>
#include <QtWidgets/QtWidgets>

namespace Internal{

ExtView::ExtView(QWidget *parent /*= Q_NULLPTR*/)
	:PaginationView(parent)
	,_model(new QStandardItemModel(0, Barcode+1, this))
{
	// setup package view info
	_model->setHeaderData(Name, Qt::Horizontal, "器械名");
	_model->setHeaderData(Doctor, Qt::Horizontal, "医生姓名");
	_model->setHeaderData(Patient, Qt::Horizontal, "病人姓名");
	_model->setHeaderData(Identifier, Qt::Horizontal, "住院号");
	_model->setHeaderData(Age, Qt::Horizontal, "年龄");
	_model->setHeaderData(Gender, Qt::Horizontal, "性别");
	_model->setHeaderData(Barcode, Qt::Horizontal, "条码");

	setModel(_model);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = horizontalHeader();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);


}

void ExtView::load(const int orderId, int page /*= 0*/) {
	clear();

	QVariantMap vmap;
	vmap.insert("ext_order_id", orderId);

	Core::app()->startWaitingOn(this);
	Url::post(Url::PATH_EXT_SEARCH, vmap, [this](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> orders = resp.getAsList("ext_info");
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();

			QList<QStandardItem *> rowItems;

			QStandardItem *item_type_name = new QStandardItem();
			item_type_name->setTextAlignment(Qt::AlignCenter);
			item_type_name->setText(map["pkg_type_name"].toString());
			item_type_name->setData(map["pkg_type_id"]);
			rowItems.append(item_type_name);

			QStandardItem *item_doctor = new QStandardItem();
			item_doctor->setTextAlignment(Qt::AlignCenter);
			item_doctor->setText(map["doctor_name"].toString());
			rowItems.append(item_doctor);

			QStandardItem *item_patient = new QStandardItem();
			item_patient->setTextAlignment(Qt::AlignCenter);
			item_patient->setText(map["patient_name"].toString());
			rowItems.append(item_patient);

			QStandardItem *item_admission = new QStandardItem();
			item_admission->setTextAlignment(Qt::AlignCenter);
			item_admission->setText(map["admission"].toString());
			rowItems.append(item_admission);

			QStandardItem *item_age = new QStandardItem();
			item_age->setTextAlignment(Qt::AlignCenter);
			item_age->setText(map["age"].toString());
			rowItems.append(item_age);

			QStandardItem *item_gender = new QStandardItem();
			item_gender->setTextAlignment(Qt::AlignCenter);
			item_gender->setText(literal_gender(map["gender"].toString()));
			rowItems.append(item_gender);

			QStandardItem *item_pkg_id = new QStandardItem();
			item_pkg_id->setTextAlignment(Qt::AlignCenter);
			item_pkg_id->setText(map["pkg_id"].toString());
			rowItems.append(item_pkg_id);

			_model->appendRow(rowItems);
		}
	});
}

ExtOrderView::ExtOrderView(QWidget *parent /*= Q_NULLPTR*/)
	:PaginationView(parent)
	, _model(new QStandardItemModel(0, RepaierPhone + 1, this))
	, _map(new QVariantMap())
{
	// setup package view info
	_model->setHeaderData(Vendor, Qt::Horizontal, "供应商");
	_model->setHeaderData(Sender, Qt::Horizontal, "送货人");
	_model->setHeaderData(SenderPhone, Qt::Horizontal, "送货人电话");
	_model->setHeaderData(Receiver, Qt::Horizontal, "接收人");
	_model->setHeaderData(ReceiveTime, Qt::Horizontal, "接收时间");
	_model->setHeaderData(Repaier, Qt::Horizontal, "归还人");
	_model->setHeaderData(RepayTime, Qt::Horizontal, "归还时间");
	_model->setHeaderData(RepaierName, Qt::Horizontal, "归还接收人");
	_model->setHeaderData(RepaierPhone, Qt::Horizontal, "归还接收人电话");
	setModel(_model);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = horizontalHeader();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);

	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onRowClicked(const QModelIndex &)));
}

void ExtOrderView::onRowClicked(const QModelIndex &index) {
	int orderId = _model->item(index.row(), 0)->data().toInt();
	QString vendor = _model->item(index.row(), 0)->text();
	QString sender = _model->item(index.row(), 1)->text();
	QString senderPhone = _model->item(index.row(), 2)->text();
	QString receiver = _model->item(index.row(), 3)->text();
	QString receiveTime = _model->item(index.row(), 4)->text();

	QString repay = _model->item(index.row(), 7)->text();
	QString repayPhone = _model->item(index.row(), 8)->text();

	_map->clear();
	_map->insert("orderId", orderId);
	_map->insert("vendor", vendor);
	_map->insert("sender", sender);
	_map->insert("senderPhone", senderPhone);
	_map->insert("receiver", receiver);
	_map->insert("receiveTime", receiveTime);
	_map->insert("repay", repay);
	_map->insert("repayPhone", repayPhone);

	emit sendOrderId(orderId);
}

void ExtOrderView::load(const QDate& fromDate, const QDate& endDate, int page /*= 0*/) {
	clear();

	QVariantMap vmap;
	//XDateScope timeScope(XPeriod::RecentWeek);
	vmap.insert("start_time", fromDate);
	vmap.insert("end_time", endDate);

	Core::app()->startWaitingOn(this);
	Url::post(Url::PATH_EXTORDER_SEARCH, vmap, [this](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> orders = resp.getAsList("ext_order_info");
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();
			
			QList<QStandardItem *> rowItems;

			QStandardItem *item_sup_name = new QStandardItem();
			item_sup_name->setTextAlignment(Qt::AlignCenter);
			item_sup_name->setText(map["sup_name"].toString());
			item_sup_name->setData(map["id"]);
			rowItems.append(item_sup_name);

			QStandardItem *item_send_name = new QStandardItem();
			item_send_name->setTextAlignment(Qt::AlignCenter);
			item_send_name->setText(map["send_name"].toString());
			rowItems.append(item_send_name);

			QStandardItem *item_send_phone = new QStandardItem();
			item_send_phone->setTextAlignment(Qt::AlignCenter);
			item_send_phone->setText(map["send_phone"].toString());
			rowItems.append(item_send_phone);

			QStandardItem *item_operator_name = new QStandardItem();
			item_operator_name->setTextAlignment(Qt::AlignCenter);
			item_operator_name->setText(map["p_operator_name"].toString());
			item_operator_name->setData(map["p_operator_id"]);
			rowItems.append(item_operator_name);

			QStandardItem *item_date = new QStandardItem();
			item_date->setTextAlignment(Qt::AlignCenter);
			item_date->setText(map["p_date"].toString());
			rowItems.append(item_date);

			QStandardItem *item_repaier_name = new QStandardItem();
			item_repaier_name->setTextAlignment(Qt::AlignCenter);
			item_repaier_name->setText(map["o_operator_name"].toString());
			item_repaier_name->setData(map["o_operator_id"]);
			rowItems.append(item_repaier_name);

			QStandardItem *item_o_date = new QStandardItem();
			item_o_date->setTextAlignment(Qt::AlignCenter);
			item_o_date->setText(map["o_date"].toString());
			rowItems.append(item_o_date);

			QStandardItem *item_repay_name = new QStandardItem();
			item_repay_name->setTextAlignment(Qt::AlignCenter);
			item_repay_name->setText(map["repay_name"].toString());
			rowItems.append(item_repay_name);

			QStandardItem *item_repay_phone = new QStandardItem();
			item_repay_phone->setTextAlignment(Qt::AlignCenter);
			item_repay_phone->setText(map["repay_phone"].toString());
			rowItems.append(item_repay_phone);

			_model->appendRow(rowItems);
		}
	});
}

} // namespace Internal

ExtManagePanel::ExtManagePanel(QWidget *parent)
	: CssdOverlayPanel(parent)
	, _view(new Internal::ExtView(this))
	, _orderView(new Internal::ExtOrderView(this))
	, _title(new Composite::Title("订单详情", false))
{
	_weekButton = new QRadioButton("本周", this);
	_monthButton = new QRadioButton("本月", this);
	_yearButton = new QRadioButton("今年", this);
	_customButton = new QRadioButton("自定义", this);

	// add radio buttons into group
	_dateButtons = new QButtonGroup(this);
	_dateButtons->addButton(_weekButton, XPeriod::ThisWeek);
	_dateButtons->addButton(_monthButton, XPeriod::ThisMonth);
	_dateButtons->addButton(_yearButton, XPeriod::ThisYear);
	_dateButtons->addButton(_customButton, XPeriod::Custom);
	connect(_dateButtons, QOverload<int, bool>::of(&QButtonGroup::buttonToggled),
		this, &ExtManagePanel::onDateButtonToggled);

	QWidget *topWidget = new QWidget(this);
	QHBoxLayout * hlayout = new QHBoxLayout(topWidget);
	hlayout->addWidget(_weekButton);
	hlayout->addWidget(_monthButton);
	hlayout->addWidget(_yearButton);
	hlayout->addWidget(_customButton);

	QDate today = QDate::currentDate();
	_startDateEdit = new QDateEdit(today, this);
	_startDateEdit->setCalendarPopup(true);
	_startDateEdit->setMaximumDate(today);
	_startDateEdit->setEnabled(false);
	hlayout->addWidget(_startDateEdit);

	hlayout->addWidget(new QLabel("-", this));

	_endDateEdit = new QDateEdit(today, this);
	_endDateEdit->setCalendarPopup(true);
	_endDateEdit->setMaximumDate(today);
	_endDateEdit->setEnabled(false);
	hlayout->addWidget(_endDateEdit);

	QToolButton *searchbtn = new QToolButton(this);
	searchbtn->setIcon(QIcon(":/res/search.png"));
	hlayout->addWidget(searchbtn);

	hlayout->addStretch(1);
	hlayout->setSpacing(13);
	_weekButton->setChecked(true);

	topWidget->setFixedHeight(50);

	QVBoxLayout *order_vlayout = new QVBoxLayout();
	order_vlayout->addWidget(topWidget);
	order_vlayout->addWidget(_orderView);
	order_vlayout->setStretch(1, 1);

	// detail panel
	QWidget *detailPanel = new QWidget(this);
	QVBoxLayout *vlayout = new QVBoxLayout(detailPanel);
	vlayout->setContentsMargins(0, 0, 0, 0);
	vlayout->setSpacing(0);
	vlayout->addWidget(_title);
	vlayout->addWidget(_view);

	// tip
	const QString text = "* 外来器械/植入物的接收和归还应及时做好登记\n"
		"* 接收后请到手术器械回收页面进行回收操作";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *receiveButton = new Ui::PrimaryButton("接收登记");
	Ui::PrimaryButton *returnButton = new Ui::PrimaryButton("归还登记");
	tip->addButton(receiveButton);
	tip->addButton(returnButton);
	connect(receiveButton, SIGNAL(clicked()), this, SLOT(addReception()));
	connect(returnButton, SIGNAL(clicked()), this, SLOT(addReturn()));
	connect(searchbtn, &QToolButton::clicked, this, &ExtManagePanel::reset);
	connect(_orderView, SIGNAL(sendOrderId(int)), this, SLOT(loadOrder(int)));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addLayout(order_vlayout);
	layout->addWidget(detailPanel);
	layout->addWidget(tip);
	layout->setStretch(0, 3);
	layout->setStretch(1, 2);

	QTimer::singleShot(0, this, &ExtManagePanel::reset);
}


void ExtManagePanel::onDateButtonToggled(int id, bool checked)
{
	if (XPeriod::Custom == id) {
		_startDateEdit->setEnabled(checked);
		_endDateEdit->setEnabled(checked);
		return;
	}

	// for other buttons, ignore their unchecked signals.
	if (!checked) return;

	XDateScope ds(static_cast<XPeriod>(id));
	_startDateEdit->setDate(ds.from);
	_endDateEdit->setDate(ds.to);
}

void ExtManagePanel::handleBarcode(const QString &) {

}

void ExtManagePanel::reset() {
	_orderView->load(_startDateEdit->date(), _endDateEdit->date());
}

void ExtManagePanel::loadOrder(int orderId) {
	_view->load(orderId);
}

void ExtManagePanel::addReception() {
	ExtRecvDialog d(this);
	if (QDialog::Accepted == d.exec()) {
		reset();
	}
}

void ExtManagePanel::addReturn() {
	const QVariantMap *map = _orderView->itemMap();
	if (map == NULL || map->isEmpty()) return;

	ExtRepayDialog d(map, this);
	if (QDialog::Accepted == d.exec()) {
		reset();
	}
}
