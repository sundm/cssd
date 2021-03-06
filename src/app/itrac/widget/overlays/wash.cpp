#include "wash.h"
#include "barcode.h"
#include "devicewidget.h"
#include "tips.h"
#include "ui/buttons.h"
#include "widget/controls/plateview.h"
#include "dialog/regexpinputdialog.h"
#include "core/net/url.h"
#include "dialog/operatorchooser.h"
#include "xnotifier.h"
#include "QtWidgets/QtWidgets"

WashPanel::WashPanel(QWidget *parent)
	: CssdOverlayPanel(parent) {

	_deviceArea = new DeviceArea;
	_deviceArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

	_plateView = new PlateView;

	const QString text = "1 选择清洗机，然后选择清洗程序\n2 扫描或输入托盘条码\n3 开始清洗"
		"\n\n注意：请确保托盘内的物品能全部进入清洗机";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("开始清洗");
	tip->addQr();
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));
	

	Ui::IconButton *addPlateButton = new Ui::IconButton(":/res/fill-plate-24.png");
	addPlateButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	connect(addPlateButton, SIGNAL(clicked()), this, SLOT(addPlate()));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_deviceArea, 0, 0);
	layout->addWidget(addPlateButton, 1, 0);
	layout->addWidget(_plateView, 2, 0);
	layout->addWidget(tip, 0, 1, 3, 1);
	layout->setRowStretch(2, 1);

	QTimer::singleShot(200, [this] { _deviceArea->load(itrac::DeviceType::Washer); });
}

void WashPanel::addPlate() {
	bool ok;
	QRegExp regExp("\\d{8,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入网篮条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void WashPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Plate) {
		int id = bc.intValue();
		if (!_plateView->hasPlate(id)) {

			_plateView->addPlate(id);
		}
	}
	else if (bc.type() == Barcode::Device)
	{
		//todo
		_deviceArea->scanDevice(bc.intValue());
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		commit();
	}
}

void WashPanel::commit() {
	DeviceItem *item = _deviceArea->currentItem();
	if (!item || item->isRunning())
	{
		XNotifier::warn(QString("请选择清洗机"));
		return;
	}
	
	//if (item->isRunning())
	//{
	//	QMessageBox msgBox;
	//	msgBox.setText(QString("是否确认在已经开始的设备中添加？").arg(item->name()));
	//	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	//	msgBox.setDefaultButton(QMessageBox::Ok);
	//	if (msgBox.exec() != QMessageBox::Ok) return;
	//}

	int programId = item->programId();
	if (0 == programId) {
		XNotifier::warn(QString("请选择清洗机程序"));
		return;
	}

	int deviceId = item->id();

	QVariantList plates = _plateView->plates();
	if (plates.isEmpty()) {
		XNotifier::warn(QString("网篮为空，无法完成清洗登记"));
		return;
	}

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) {
		XNotifier::warn(QString("请选择操作员"));
		return;
	}

	QVariantMap vmap;
	vmap.insert("plate_ids", plates);
	vmap.insert("device_id", deviceId);
	vmap.insert("program_id", programId);
	vmap.insert("operator_id", opId);

	post(url(PATH_WASH_ADD), vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success())
			XNotifier::warn(QString("无法完成清洗登记: ").append(resp.errorString()));
		else {
			XNotifier::warn("已完成清洗登记");
			_deviceArea->currentItem()->setRunning();
			reset();
		}
	});
}

void WashPanel::reset() {
	_plateView->clear();
	_deviceArea->load(itrac::DeviceType::Washer);
}
