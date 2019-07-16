#include "tracepage.h"
#include "barcode.h"
#include "core/net/url.h"
#include "xnotifier.h"

#include <ui/ui_commons.h>
#include <xui/searchedit.h>
#include <QtWidgets/QtWidgets>

TracePage::TracePage(QWidget *parent)
	: QWidget(parent), _searchBox(new SearchEdit), _grid(new QGridLayout)
{
	_grid->setSpacing(100);
	_searchBox->setPlaceholderText("物品包ID/病人ID");
	_searchBox->setMinimumHeight(36);
	connect(_searchBox, &QLineEdit::returnPressed, this, &TracePage::startTrace);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addStretch(0);
	hLayout->addWidget(_searchBox);
	hLayout->addStretch(0);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(100, 40, 100, 100);
	layout->addLayout(hLayout);
	layout->addLayout(_grid);
}

void TracePage::handleBarcode(const QString &code) {
	_searchBox->setText(code);
	Barcode(code).type() == Barcode::Package ?
		tracePackage(code) :
		tracePatient(code);
}

void TracePage::clear() {
	QLayoutItem *child;
	while ((child = _grid->takeAt(1)) != 0) {
		delete child;
	}
}

void TracePage::startTrace() {
	handleBarcode(_searchBox->text());
}

void TracePage::tracePackage(const QString &id) {
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
		
		/*model->insertRows(0, pkgs.count());
		for (int i = 0; i != pkgs.count(); ++i) {
			QVariantMap map = pkgs[i].toMap();
			_model->setData(_model->index(i, Name), map["instrument_name"]);
			_model->setData(_model->index(i, Id), map["instrument_id"]);
			_model->setData(_model->index(i, Vip), Internal::getVipLiteral(map["is_vip_instrument"].toString()));
			_model->setData(_model->index(i, Pinyin), map["pinyin_code"]);
		}*/
	});
}

void TracePage::tracePatient(const QString &) {

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
