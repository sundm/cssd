#include "repayextdialog.h"
#include "xnotifier.h"
#include "core/constants.h"
#include "core/application.h"
#include "core/user.h"
#include "core/net/url.h"
#include "core/inliner.h"
#include "ui/groups.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "ui/buttons.h"
#include "widget/controls/combos.h"
#include "widget/controls/idedit.h"
#include <xernel/xtimescope.h>
#include "ui/views.h"
#include "ui/composite/waitingspinner.h"
#include "addextdialog.h"
#include <QtWidgets/QtWidgets>
#include <QDateTime>
#include <QTimer>
#include <QSpinBox>

ExtRepayDialog::ExtRepayDialog(const QVariantMap *map, QWidget *parent)
	: QDialog(parent)
	, _view(new TableView)
	, _model(new QStandardItemModel(0, Barcode + 1, this))
	, _repayEdit(new Ui::FlatEdit)
	, _repayPhoneEdit(new Ui::FlatEdit)
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("外来器械归还登记");

	_model->setHeaderData(Name, Qt::Horizontal, "器械名");
	_model->setHeaderData(Doctor, Qt::Horizontal, "医生姓名");
	_model->setHeaderData(Patient, Qt::Horizontal, "病人姓名");
	_model->setHeaderData(Identifier, Qt::Horizontal, "住院号");
	_model->setHeaderData(Age, Qt::Horizontal, "年龄");
	_model->setHeaderData(Gender, Qt::Horizontal, "性别");
	_model->setHeaderData(Barcode, Qt::Horizontal, "条码");

	_view->setModel(_model);
	_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	_orderId = map->value("orderId").toInt();
	_vendor = new Ui::NormalLabel(map->value("vendor").toString());
	_sender = new Ui::NormalLabel(map->value("sender").toString());
	_senderPhone = new Ui::NormalLabel(map->value("senderPhone").toString());
	_receiver = new Ui::NormalLabel(map->value("receiver").toString());
	_receiveTime = new Ui::NormalLabel(map->value("receiveTime").toString());

	_repayEdit->setText(map->value("repay").toString());
	_repayPhoneEdit->setText(map->value("repayPhone").toString());

	FormGroup * transferGroup = new FormGroup(this);
	transferGroup->addRow("供应商", _vendor);
	transferGroup->addRow("送货人", _sender);
	transferGroup->addRow("联系电话", _senderPhone);
	transferGroup->addRow("接收人", _receiver);
	transferGroup->addRow("接收时间", _receiveTime);

	_repaier = new Ui::NormalLabel(Core::currentUser().name);
	QString currentDate = QDateTime::currentDateTime().toString("yyyy/MM/dd-HH:mm:ss");
	_repayTime = new Ui::NormalLabel(currentDate);

	FormGroup * repayGroup = new FormGroup(this);
	repayGroup->addRow("归还接收人（*）", _repayEdit);
	repayGroup->addRow("归还接收人电话（*）", _repayPhoneEdit);
	repayGroup->addRow("归还操作员", _repaier);
	repayGroup->addRow("归还时间", _repayTime);
	
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("确认归还", Ui::BtnSize::Small);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(transferGroup);
	layout->addWidget(_view);
	layout->addWidget(repayGroup);
	layout->addWidget(commitButton);

	resize(parent ? parent->width() / 2 : 720, sizeHint().height() * 1.5);

	QTimer::singleShot(0, this, &ExtRepayDialog::load);
}

void ExtRepayDialog::load() {
	_view->clear();

	QVariantMap vmap;
	vmap.insert("ext_order_id", _orderId);

	_waiter->start();
	post(url(PATH_EXT_SEARCH), vmap, [this](QNetworkReply *reply) {
		_waiter->stop();
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

void ExtRepayDialog::accept() {
	QString repay = _repayEdit->text();
	QString repayPhone = _repayPhoneEdit->text();
	int repaierId = Core::currentUser().id;
	QString repaier = Core::currentUser().name;

	if (repay.isEmpty()) {
		_repayEdit->setFocus();
		return;
	}

	if (repayPhone.isEmpty()) {
		_repayPhoneEdit->setFocus();
		return;
	}

	QVariantMap data;
	data.insert("id", _orderId);
	data.insert("o_operator_id", repaierId);
	data.insert("o_operator_name", repaier);
	data.insert("repay_name", repay);
	data.insert("repay_phone", repayPhone);

	post(url(PATH_EXT_RETURN), data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法提交归还登记: ").append(resp.errorString()));
			return;
		}
		QDialog::accept();
	});
}