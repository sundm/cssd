#include "tracepackagepage.h"
#include "barcode.h"
#include "core/net/url.h"
#include "xnotifier.h"

#include <ui/ui_commons.h>
#include <xui/searchedit.h>
#include <QtWidgets/QtWidgets>

TracePackagePage::TracePackagePage(QWidget *parent)
	: QWidget(parent), 
	_searchBox(new SearchEdit)
	, _map(new QVariantMap())
	, _grid(new QGridLayout)
{
	_grid->setSpacing(50);
	_searchBox->setPlaceholderText("物品包ID");
	_searchBox->setMinimumHeight(36);
	connect(_searchBox, &QLineEdit::returnPressed, this, &TracePackagePage::startTrace);

	QHBoxLayout *h_sLayout = new QHBoxLayout;
	h_sLayout->addStretch(0);
	h_sLayout->addWidget(_searchBox);
	h_sLayout->addStretch(0);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(100, 40, 100, 100);
	layout->addLayout(h_sLayout);
	layout->addLayout(_grid);

	QTimer::singleShot(200, this, SLOT(loadExtOrder()));
}

void TracePackagePage::loadExtOrder()
{
	QVariantMap vmap;
	vmap.insert("state", 1);

	post(url(PATH_EXT_SEARCH), vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
			return;
		}

		_map->clear();

		QVariantList extInfoList = resp.getAsList("ext_info");

		if (!extInfoList.isEmpty())
		{
			for (auto &extInfo : extInfoList) {
				QVariantMap map = extInfo.toMap();

				QString pkg_id = map["pkg_id"].toString();
				
				_map->insert(pkg_id, map);
			}
		}
	});

}

void TracePackagePage::handleBarcode(const QString &code) {
	_searchBox->setText(code);
	if (Barcode(code).type() == Barcode::Package)
		tracePackage(code);		
}

void TracePackagePage::clear() {
	QLayoutItem *child;
	while ((child = _grid->takeAt(0)) != 0) {
		if (child->widget()) {
			delete child->widget();
		}
		delete child;
	}
}

void TracePackagePage::startTrace() {
	handleBarcode(_searchBox->text());
}

void TracePackagePage::tracePackage(const QString &id) {
	if (id.left(4).compare(QString("1003")) == 0)
	{
		if (_map->contains(id)) {
			QVariantMap map = _map->value(id).toMap();
			int ext_id = map["id"].toInt();

			QVariantMap vmap;
			vmap.insert("id", ext_id);
			vmap.insert("pkg_id", id);

			post(url(PATH_EXT_PROCESS), vmap, [this](QNetworkReply *reply) {
				JsonHttpResponse resp(reply);
				if (!resp.success()) {
					XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
					return;
				}

				clear();
				QVariantMap &recycle_fist = resp.getAsDict("recycle_first");
				TraceItem *item = new TraceItem("第一次回收");
				item->addEntry("时间", recycle_fist["recycle_time"].toString());
				item->addEntry("回收人", recycle_fist["operator"].toString());
				_grid->addWidget(item, 0, 0);

				QVariantMap &wash = resp.getAsDict("wash_info");
				item = new TraceItem("清洗");
				item->addEntry("时间", wash["wash_start_time"].toString());
				item->addEntry("设备", wash["device_name"].toString());
				item->addEntry("程序", wash["program_name"].toString());
				item->addEntry("锅次", wash["wash_cycle"].toString());
				item->addEntry("清洗员", wash["operator"].toString());
				_grid->addWidget(item, 0, 1);

				QVariantMap &pack = resp.getAsDict("pack_trace");
				item = new TraceItem("打包");
				item->addEntry("时间", pack["pack_time"].toString());
				item->addEntry("配包人", pack["operator"].toString());
				item->addEntry("审核人", pack["pack_check_operator"].toString());
				_grid->addWidget(item, 0, 2);

				QVariantMap &ste = resp.getAsDict("sterilization_trace");
				item = new TraceItem("灭菌");
				item->addEntry("时间", ste["sterilization_start_time"].toString());
				item->addEntry("设备", ste["device_name"].toString());
				item->addEntry("程序", ste["program_name"].toString());
				item->addEntry("锅次", ste["sterilization_cycle"].toString());
				item->addEntry("灭菌员", ste["operator"].toString());
				_grid->addWidget(item, 1, 0);

				item = new TraceItem("灭菌审核");
				item->addEntry("物理监测审核时间", ste["physical_test_time"].toString());
				item->addEntry("物理监测审核人员", ste["physical_test_operator"].toString());
				item->addEntry("物理监测审核结果", ste["physical_test_result"].toString());
				item->addEntry("化学监测审核时间", ste["chemistry_test_time"].toString());
				item->addEntry("化学监测审核人员", ste["chemistry_test_operator"].toString());
				item->addEntry("化学监测审核结果", ste["chemistry_test_result"].toString());
				item->addEntry("生物监测审核时间", ste["biology_test_time"].toString());
				item->addEntry("生物监测审核人员", ste["biology_test_operator"].toString());
				item->addEntry("生物监测审核结果", ste["biology_test_result"].toString());
				_grid->addWidget(item, 1, 1);

				QVariantMap &issue = resp.getAsDict("issue_trace");
				item = new TraceItem("发放");
				item->addEntry("时间", issue["issue_time"].toString());
				item->addEntry("发放者", issue["issue_operator"].toString());
				_grid->addWidget(item, 1, 2);

				QVariantMap &recycle = resp.getAsDict("recycle_trace");
				item = new TraceItem("回收");
				item->addEntry("时间", recycle["recycle_time"].toString());
				item->addEntry("回收人", recycle["operator"].toString());
				_grid->addWidget(item, 2, 0);

				QVariantMap &wash_trace = resp.getAsDict("wash_trace");
				item = new TraceItem("第二次清洗");
				item->addEntry("时间", wash_trace["wash_start_time"].toString());
				item->addEntry("设备", wash_trace["device_name"].toString());
				item->addEntry("程序", wash_trace["program_name"].toString());
				item->addEntry("锅次", wash_trace["wash_cycle"].toString());
				item->addEntry("清洗员", wash_trace["operator"].toString());
				_grid->addWidget(item, 2, 1);
			});
		}
	}
	else {
		QString data = QString("{\"package_id\":\"%1\"}").arg(id);
		post(url(PATH_TRACE_PACKAGE), QByteArray().append(data), [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
				return;
			}
			clear();
			QVariantMap &wash = resp.getAsDict("wash_info");
			TraceItem *item = new TraceItem("清洗");
			item->addEntry("时间", wash["wash_start_time"].toString());
			item->addEntry("设备", wash["device_name"].toString());
			item->addEntry("程序", wash["program_name"].toString());
			item->addEntry("锅次", wash["wash_cycle"].toString());
			item->addEntry("清洗员", wash["operator"].toString());
			_grid->addWidget(item, 0, 0);

			QVariantMap &pack = resp.getAsDict("pack_trace");
			item = new TraceItem("打包");
			item->addEntry("时间", pack["pack_time"].toString());
			item->addEntry("配包人", pack["operator"].toString());
			item->addEntry("审核人", pack["pack_check_operator"].toString());
			_grid->addWidget(item, 0, 1);

			QVariantMap &ste = resp.getAsDict("sterilization_trace");
			item = new TraceItem("灭菌");
			item->addEntry("时间", ste["sterilization_start_time"].toString());
			item->addEntry("设备", ste["device_name"].toString());
			item->addEntry("程序", ste["program_name"].toString());
			item->addEntry("锅次", ste["sterilization_cycle"].toString());
			item->addEntry("灭菌员", ste["operator"].toString());
			_grid->addWidget(item, 0, 2);

			item = new TraceItem("灭菌审核");
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
			item = new TraceItem("发放");
			item->addEntry("时间", issue["issue_time"].toString());
			item->addEntry("发放者", issue["issue_operator"].toString());
			_grid->addWidget(item, 1, 1);

			QVariantMap &recycle = resp.getAsDict("recycle_trace");
			item = new TraceItem("回收");
			item->addEntry("时间", recycle["recycle_time"].toString());
			item->addEntry("回收人", recycle["operator"].toString());
			_grid->addWidget(item, 1, 2);
		});
	}
	
}

TraceItem::TraceItem(const QString &title, QWidget *parent /*= Q_NULLPTR*/)
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

void TraceItem::addEntry(const QString &label, const QString &field) {
	_formLayout->addRow(label + ":", new QLabel(field));
}
