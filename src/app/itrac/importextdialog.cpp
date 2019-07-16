#include "importextdialog.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include <xernel/xtimescope.h>
#include <QVBoxLayout>
#include <QStandardItemModel>

ImportExtDialog::ImportExtDialog(QWidget *parent)
	: QDialog(parent)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0,4,_view))
	, _detailView(new TableView(this))
	, _detailModel(new QStandardItemModel(0, 5, _view))
{
	_model->setHeaderData(0, Qt::Horizontal, "供应商");
	_model->setHeaderData(1, Qt::Horizontal, "送货人");
	_model->setHeaderData(2, Qt::Horizontal, "接收人");
	_model->setHeaderData(3, Qt::Horizontal, "接收时间");
	_view->setModel(_model);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_view->setSelectionMode(QAbstractItemView::SingleSelection);

	_detailModel->setHeaderData(0, Qt::Horizontal, "器械名");
	_detailModel->setHeaderData(1, Qt::Horizontal, "医生姓名");
	_detailModel->setHeaderData(2, Qt::Horizontal, "病人姓名");
	_detailModel->setHeaderData(3, Qt::Horizontal, "住院号");
	_detailModel->setHeaderData(4, Qt::Horizontal, "状态");

	_detailView->setModel(_detailModel);
	_detailView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_detailView->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(_view, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onRowClicked(const QModelIndex &)));

	QHBoxLayout *hlayout = new QHBoxLayout();
	hlayout->addWidget(_view);
	hlayout->addWidget(_detailView);

	Ui::PrimaryButton *okButton = new Ui::PrimaryButton("确定");
	connect(okButton, &QPushButton::clicked, this, &ImportExtDialog::accept);

	QVBoxLayout *vlayout = new QVBoxLayout(this);
	vlayout->addLayout(hlayout);
	vlayout->addWidget(okButton);

	resize(parent->width()/2, sizeHint().height());
	
	loadExt();
}

ImportExtDialog::~ImportExtDialog() {
}

void ImportExtDialog::onRowClicked(const QModelIndex &index) {

	int orderId = _model->item(index.row(), 0)->data().toInt();

	_detailView->clear();

	QVariantMap vmap;
	vmap.insert("ext_order_id", orderId);

	Core::app()->startWaitingOn(this);
	post(url(PATH_EXT_SEARCH), vmap, [this](QNetworkReply *reply) {
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
			item_type_name->setData(map["id"]);
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

			QStandardItem *item_state = new QStandardItem();
			item_state->setTextAlignment(Qt::AlignCenter);
			if (0 == map["state"].toInt())
				item_state->setText("未装篮");
			else
				item_state->setText("已装篮");
			item_state->setData(map["state"].toInt());
			rowItems.append(item_state);

			_detailModel->appendRow(rowItems);
		}
	});
}

void ImportExtDialog::accept() {
	QItemSelectionModel *selModel = _detailView->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	for each (QModelIndex index in indexes)
	{
		int state = _detailModel->item(index.row(), 4)->data().toInt();
		if (0 == state)
		{
			QString pkgId = _detailModel->item(index.row(), 0)->data().toString();
			QString pkgName = _detailModel->item(index.row(), 0)->text();
			emit extPkgImport(pkgId, pkgName);
			QDialog::accept();
		}
		else
			XNotifier::warn(QString("该器械已装篮，请勿重复添加"));
		
	}
	
}

void ImportExtDialog::loadExt() {
	_view->clear();

	QVariantMap vmap;
	XDateScope timeScope(XPeriod::RecentWeek);
	vmap.insert("start_time", timeScope.from);
	vmap.insert("end_time", timeScope.to);

	post(url(PATH_EXTORDER_SEARCH), vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
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

			QStandardItem *item_operator_name = new QStandardItem();
			item_operator_name->setTextAlignment(Qt::AlignCenter);
			item_operator_name->setText(map["p_operator_name"].toString());
			item_operator_name->setData(map["p_operator_id"]);
			rowItems.append(item_operator_name);

			QStandardItem *item_date = new QStandardItem();
			item_date->setTextAlignment(Qt::AlignCenter);
			item_date->setText(map["p_date"].toString());
			rowItems.append(item_date);

			_model->appendRow(rowItems);
		}
	});
}
