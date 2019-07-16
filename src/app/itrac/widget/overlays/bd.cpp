#include "bd.h"
#include "devicewidget.h"
#include "tips.h"
#include "xnotifier.h"
#include "barcode.h"

#include "core/net/url.h"
#include "ui/buttons.h"
#include "dialog/operatorchooser.h"

#include <QtWidgets/QtWidgets>


BDPanel::BDPanel(QWidget *parent)
	: CssdOverlayPanel(parent)
{
	_deviceArea = new DeviceArea;

	const QString text = "1 选择灭菌器，或扫描灭菌器条码\n2 登记BD测试结果"
		"\n\n注意：灭菌器每日第一锅必须为BD测试并做好登记，否则无法灭菌";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *qualifiedButton = new Ui::PrimaryButton("合格");
	tip->addButton(qualifiedButton);
	connect(qualifiedButton, SIGNAL(clicked()), this, SLOT(commitQualified()));

	Ui::PrimaryButton *unqualifiedButton = new Ui::PrimaryButton("不合格");
	tip->addButton(unqualifiedButton);
	connect(unqualifiedButton, SIGNAL(clicked()), this, SLOT(commitUnqualified()));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(_deviceArea);
	layout->addWidget(tip);

	QTimer::singleShot(200, [this] { _deviceArea->load(itrac::DeviceType::Sterilizer); });
}

void BDPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Device) {
		// TODO
	}
}

void BDPanel::commitQualified() {
	commit(true);
}

void BDPanel::commitUnqualified() {
	commit(false);
}

void BDPanel::commit(bool ok) {
	DeviceItem *item = _deviceArea->currentItem();
	if (!item) return;

	int deviceId = item->id();

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	// ready
	QVariantMap vmap;
	vmap.insert("device_id", deviceId);
	vmap.insert("test_result", ok ? "1" : "0");
	vmap.insert("operator_id", opId);

	Url::post(Url::PATH_BD_ADD, vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("提交BD登记失败: ").append(resp.errorString()));
		}
		else {
			XNotifier::warn(QString("已提交BD测试结果"));
		}
	});
}
