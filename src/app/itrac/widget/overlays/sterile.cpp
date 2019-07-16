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
#include "dialog/regexpinputdialog.h"

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

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "手工添加");
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *minusButton = new Ui::IconButton(":/res/delete-24.png", "删除选中");
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(addButton);
	hLayout->addWidget(minusButton);
	hLayout->addStretch();

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_deviceArea, 0, 0);
	layout->addLayout(hLayout, 1, 0);
	layout->addWidget(_pkgView, 2, 0);
	layout->addWidget(tip, 0, 1, 3, 1);
	layout->setRowStretch(2, 1);

	//_deviceArea->load();
	QTimer::singleShot(200, [this] { _deviceArea->load(itrac::DeviceType::Sterilizer); });
}

void SterilePanel::addEntry() {
	bool ok;
	QRegExp regExp("\\d{10,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入包条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void SterilePanel::removeEntry() {
	QItemSelectionModel *selModel = _pkgView->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_pkgView->model()->removeRow(indexes.at(i - 1).row());
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

	post(url(PATH_STERILE_ADD), vmap, [=](QNetworkReply *reply) {
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
		reset();
	});
}

void SterilePanel::reset() {
	_pkgView->clear();
	_deviceArea->load(itrac::DeviceType::Sterilizer);
}
