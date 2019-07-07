#include "pack.h"
#include "barcode.h"
#include "devicewidget.h"
#include "tips.h"
#include "ui/buttons.h"
#include "widget/controls/plateview.h"
#include "dialog/operatorchooser.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include <printer/labelprinter.h>
#include <QtWidgets/QtWidgets>

PackPanel::PackPanel(QWidget *parent) : CssdOverlayPanel(parent) {
	_plateView = new PackPlateView;
	//connect(_plateView, &PackPlateView::packed, this, &PackPanel::print);

	const QString text = "1 扫描或输入托盘条码\n2 开始配包"
		"\n\n注意：请根据当前标签打印机选择对应的物品托盘";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("开始配包");
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_plateView, 0, 0);
	layout->addWidget(tip, 0, 1);
}

void PackPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Plate) {
		int id = bc.intValue();
		//if (!_plateView->hasPlate(id)) {
		_plateView->addPlate(id);
		//}
	}
}

void PackPanel::commit() {
	QVariantList plates = _plateView->plates();
	if (plates.isEmpty()) return;

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	int checkerId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	_plateView->doPack(opId, checkerId);
}

//void PackPanel::print(const std::list<std::list<std::string>> & labels) {
//	Printer *printer = PrinterFactory().Create(PrinterFactory::ZEBRA_GT8);
//	if (0 != printer->open()) {
//		XNotifier::warn(this, "启动标签打印机失败，稍后可到历史查询页中重打标签", -1);
//		return;
//	}
//	printer->printLabels("label/packageLabel", labels);
//	printer->close();
//	delete printer;
//}
