#include "ext.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/net/url.h"
#include "core/inliner.h"
#include "dialog/extdialog.h"
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
	_model->setHeaderData(Name, Qt::Horizontal, "器械名称");
	_model->setHeaderData(Vendor, Qt::Horizontal, "供应商");
	_model->setHeaderData(Sender, Qt::Horizontal, "送货人");
	_model->setHeaderData(SenderPhone, Qt::Horizontal, "送货人电话");
	_model->setHeaderData(Receiver, Qt::Horizontal, "接收人");
	_model->setHeaderData(ReceiveTime, Qt::Horizontal, "接收时间");
	_model->setHeaderData(Patient, Qt::Horizontal, "患者姓名");
	_model->setHeaderData(Gender, Qt::Horizontal, "性别");
	_model->setHeaderData(Age, Qt::Horizontal, "年龄");
	_model->setHeaderData(Identifier, Qt::Horizontal, "住院号");
	_model->setHeaderData(Doctor, Qt::Horizontal, "医生");
	_model->setHeaderData(Barcode, Qt::Horizontal, "条码");
	setModel(_model);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = horizontalHeader();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void ExtView::load(int page /*= 0*/) {
	clear();

	QVariantMap vmap;
	XDateScope timeScope(XPeriod::RecentWeek);
	vmap.insert("start_time", timeScope.from);
	vmap.insert("end_time", timeScope.to);

	Core::app()->startWaitingOn(this);
	Url::post(Url::PATH_EXT_SEARCH, vmap, [this](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> orders = resp.getAsList("ext_info");
		_model->insertRows(0, orders.count());
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();
			_model->setData(_model->index(i, Name), map["pkg_type_name"]);
			_model->setData(_model->index(i, Vendor), map["sup_name"]);
			_model->setData(_model->index(i, Sender), map["send_name"]);
			_model->setData(_model->index(i, SenderPhone), map["send_phone"]);
			_model->setData(_model->index(i, Receiver), map["p_operator_name"]);
			_model->setData(_model->index(i, ReceiveTime), map["p_date"]);
			_model->setData(_model->index(i, Patient), map["patient_name"]);
			_model->setData(_model->index(i, Gender), literal_gender(map["gender"].toString()));
			_model->setData(_model->index(i, Age), map["age"]);
			_model->setData(_model->index(i, Identifier), map["admission"]);
			_model->setData(_model->index(i, Doctor), map["doctor_name"]);
			_model->setData(_model->index(i, Barcode), map["pkg_id"]);
		}
	});
}

} // namespace Internal

ExtManagePanel::ExtManagePanel(QWidget *parent)
	: CssdOverlayPanel(parent)
	, _view(new Internal::ExtView(this))
{
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

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(_view);
	layout->addWidget(tip);

	QTimer::singleShot(0, this, &ExtManagePanel::reset);
}

void ExtManagePanel::handleBarcode(const QString &) {

}

void ExtManagePanel::reset() {
	_view->load();
}

void ExtManagePanel::addReception() {
	ExtRecvDialog d(this);
	if (QDialog::Accepted == d.exec()) {
		reset();
	}
}

void ExtManagePanel::addReturn() {

}
