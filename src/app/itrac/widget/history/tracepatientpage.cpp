#include "tracepatientpage.h"
#include "barcode.h"
#include "core/net/url.h"
#include "xnotifier.h"

#include <ui/ui_commons.h>
#include <xui/searchedit.h>
#include <QtWidgets/QtWidgets>

TracePatientPage::TracePatientPage(QWidget *parent)
	: QWidget(parent), 
	_searchBox(new SearchEdit), 
	_grid(new QGridLayout),
	_view(new QTableView),
	_model(new QStandardItemModel(0, 3, _view))
{
	_grid->setSpacing(100);
	_searchBox->setPlaceholderText("患者ID");
	_searchBox->setMinimumHeight(36);
	connect(_searchBox, &QLineEdit::returnPressed, this, &TracePatientPage::startTrace);

	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addWidget(_searchBox);
	vLayout->addWidget(_view);
	_view->setHidden(true);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(100, 40, 100, 100);
	layout->addLayout(vLayout);
	layout->addLayout(_grid);

	layout->setStretch(0, 2);
	layout->setStretch(1, 5);

	_model->setHeaderData(0, Qt::Horizontal, "包ID");
	_model->setHeaderData(1, Qt::Horizontal, "包名");
	_model->setHeaderData(2, Qt::Horizontal, "使用日期");

	_view->setModel(_model);
	_view->setSelectionMode(QAbstractItemView::SingleSelection);
	_view->setSelectionBehavior(QAbstractItemView::SelectRows);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	_view->horizontalHeader()->setStretchLastSection(true);

	connect(_view, SIGNAL(clicked(const QModelIndex &)), this, SLOT(showDetail(const QModelIndex &)));
}

void TracePatientPage::handleBarcode(const QString &code) {
	_searchBox->setText(code);
	if (Barcode(code).type() == Barcode::Unknown)
		tracePatient(code);		
}

void TracePatientPage::clear() {
	QLayoutItem *child;
	while ((child = _grid->takeAt(0)) != 0) {
		if (child->widget()) {
			delete child->widget();
		}
		delete child;
	}
}

void TracePatientPage::startTrace() {
	handleBarcode(_searchBox->text());
}

void TracePatientPage::tracePackage(const QString &id) {
	QString data = QString("{\"package_id\":\"%1\"}").arg(id);
	post(url(PATH_TRACE_PACKAGE), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
			return;
		}

		clear();

		QVariantMap &wash = resp.getAsDict("wash_info");
		TracePaientItem *item = new TracePaientItem("清洗");
		item->addEntry("时间", wash["wash_start_time"].toString());
		item->addEntry("设备", wash["device_name"].toString());
		item->addEntry("程序", wash["program_name"].toString());
		item->addEntry("锅次", wash["wash_cycle"].toString());
		item->addEntry("清洗员", wash["operator"].toString());
		_grid->addWidget(item, 0, 0);

		QVariantMap &pack = resp.getAsDict("pack_trace");
		item = new TracePaientItem("打包");
		item->addEntry("时间", pack["pack_time"].toString());
		item->addEntry("配包人", pack["operator"].toString());
		item->addEntry("审核人", pack["pack_check_operator"].toString());
		_grid->addWidget(item, 0, 1);

		QVariantMap &ste = resp.getAsDict("sterilization_trace");
		item = new TracePaientItem("灭菌");
		item->addEntry("时间", ste["sterilization_start_time"].toString());
		item->addEntry("设备", ste["device_name"].toString());
		item->addEntry("程序", ste["program_name"].toString());
		item->addEntry("锅次", ste["sterilization_cycle"].toString());
		item->addEntry("灭菌员", ste["operator"].toString());
		_grid->addWidget(item, 0, 2);

		item = new TracePaientItem("灭菌审核");
		item->addEntry("物理监测审核时间", ste["physical_test_time"].toString());
		item->addEntry("物理监测审核人员", ste["physical_test_operator"].toString());
		item->addEntry("物理监测审核结果", ste["physical_test_result"].toString());
		item->addEntry("化学监测审核时间", ste["chemistry_test_time"].toString());
		item->addEntry("化学监测审核人员", ste["chemistry_test_operator"].toString());
		item->addEntry("化学监测审核结果", ste["chemistry_test_result"].toString());
		item->addEntry("生物监测审核时间", ste["biology_test_time"].toString());
		item->addEntry("生物监测审核人员", ste["biology_test_operator"].toString());
		item->addEntry("生物监测审核结果", ste["biology_test_result"].toString());
		_grid->addWidget(item, 1, 0);

		QVariantMap &issue = resp.getAsDict("issue_trace");
		item = new TracePaientItem("发放");
		item->addEntry("时间", issue["issue_time"].toString());
		item->addEntry("发放者", issue["issue_operator"].toString());
		_grid->addWidget(item, 1, 1);

		QVariantMap &recycle = resp.getAsDict("recycle_trace");
		item = new TracePaientItem("回收");
		item->addEntry("时间", recycle["recycle_time"].toString());
		item->addEntry("回收人", recycle["operator"].toString());
		_grid->addWidget(item, 1, 2);
	
	});
}

void TracePatientPage::showDetail(const QModelIndex &index) {
	int row = index.row();
	QString orderId = _model->data(_model->index(row, 0)).toString();
	tracePackage(orderId);
}

void TracePatientPage::tracePatient(const QString &id) {
	QString data = QString("{\"patient_id\":\"%1\"}").arg(id);
	post(url(PATH_TRACE_PATIENT), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
			clear();
			_model->removeRows(0, _model->rowCount());
			_view->setHidden(true);
			return;
		}

		clear();
		_model->removeRows(0, _model->rowCount());

		QList<QVariant> &traces = resp.getAsList("package_traces");
		if (0 == traces.count()){
			XNotifier::warn(QString("查询失败: 未找到该记录号!"));
			_view->setHidden(true);
			return;
		}
		else
			_view->setHidden(false);


		_model->insertRows(0, traces.count());
		QString firstId;
		for (int i = 0; i != traces.count(); ++i) {
			QVariantMap map = traces[i].toMap();
			if (i == 0)	firstId = map["package_id"].toString();
			_model->setData(_model->index(i, 0), map["package_id"]);
			_model->setData(_model->index(i, 1), map["package_name"]);
			_model->setData(_model->index(i, 2), map["use_time"]);
		}
		
		tracePackage(firstId);
		_view->selectRow(0);
	});
}

TracePaientItem::TracePaientItem(const QString &title, QWidget *parent /*= Q_NULLPTR*/)
	:QGroupBox(parent), _formLayout(new QFormLayout)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	QLabel *titleLabel = new QLabel(title);
	titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	titleLabel->setAlignment(Qt::AlignCenter);

	layout->addWidget(titleLabel);
	
	QWidget *line = Ui::createSeperator(Qt::Horizontal);
	layout->addWidget(line);
	layout->addLayout(_formLayout);
}

void TracePaientItem::addEntry(const QString &label, const QString &field) {
	_formLayout->addRow(label + ":", new QLabel(field));
}
