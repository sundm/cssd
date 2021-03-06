#include "sterile.h"
#include "devicewidget.h"
#include "tips.h"
#include "xnotifier.h"
#include "barcode.h"

#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/inputfields.h"
#include "widget/controls/packageview.h"
#include "dialog/operatorchooser.h"

#include "util/printermanager.h"
#include <printer/labelprinter.h>
#include <QtWidgets/QtWidgets>


SterilePanel::SterilePanel(QWidget *parent)
	: CssdOverlayPanel(parent)
{
	_deviceArea = new DeviceArea;

	_pkgView = new SterilePackageView;

	const QString text = "1 选择灭菌器，然后选择灭菌程序\n2 扫描或输入物品包条码\n3 开始灭菌"
		"\n\n注意：请确保扫描的物品能全部进入所选灭菌机";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("开始灭菌");
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_deviceArea, 0, 0);
	layout->addWidget(_pkgView, 1, 0);
	layout->addWidget(tip, 0, 1, 2, 1);
	layout->setRowStretch(1, 1);

	//_deviceArea->load();
	QTimer::singleShot(200, [this] { _deviceArea->load(DeviceArea::Sterilizer); });
}

void SterilePanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Package && !_pkgView->hasPackage(code)) {
		_pkgView->addPackage(code);
	}
}

void SterilePanel::commit() {
	DeviceItem *item = _deviceArea->currentItem();
	if (!item) return;

	int programId = item->programId();
	if (0 == programId) return;

	int deviceId = item->id();

	QVariantList packages = _pkgView->packages();
	if (packages.isEmpty()) return;

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	// check printer
	LabelPrinter *printer = PrinterManager::currentPrinter();
	if (0 != printer->open("ZDesigner GT800 (ZPL)")) {
		XNotifier::warn("打印机未就绪");
		return;
	}

	// ready
	QVariantMap vmap;
	vmap.insert("package_ids", packages);
	vmap.insert("device_id", deviceId);
	vmap.insert("program_id", programId);
	vmap.insert("operator_id", opId);

	bool chemInvolved = true; //TODO
	bool bioInvolved = false;
	vmap.insert("chemical_test", chemInvolved);
	vmap.insert("biological_test", bioInvolved);

	Url::post(Url::PATH_STERILE_ADD, vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			printer->close();
			XNotifier::warn(QString("提交灭菌登记失败: ").append(resp.errorString()));
			return;
		}

		QStringList datetimes = resp.getAsString("sterilize_time").split(" ");
		if (datetimes.length() < 2) {
			XNotifier::warn("返回时间错误:");
			return;
		}

		SterilizeLabel label;
		label.sterilizeId = resp.getAsString("test_id");
		label.sterilizeName = item->name();
		label.sterilizeDate = datetimes[0];
		label.sterilizeTime = datetimes[1];
		label.panNum = item->cycle() + 1;
		label.packageNum = packages.count();

		printer->printSterilizedLabel(label);
		printer->close();

		XNotifier::warn("已完成灭菌登记");
	});
}
