#include "pack.h"
#include "barcode.h"
#include "devicewidget.h"
#include "tips.h"
#include "ui/buttons.h"
#include "widget/controls/plateview.h"
#include "controls/packageview.h"
#include "dialog/regexpinputdialog.h"
#include "dialog/operatorchooser.h"
#include "dialog/reprintdialog.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include <printer/labelprinter.h>
#include <QtWidgets/QtWidgets>

PackPanel::PackPanel(QWidget *parent) : CssdOverlayPanel(parent) {
	_plateView = new PackPlateView;
	_detailView = new PackageDetailView;
	//connect(_plateView, &PackPlateView::packed, this, &PackPanel::print);

	const QString text = "1 扫描或输入托盘条码\n2 开始配包"
		"\n\n注意：请根据当前标签打印机选择对应的物品托盘";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("开始配包");
	Ui::PrimaryButton *rePrintButton = new Ui::PrimaryButton("重新打印");
	tip->addQr();
	tip->addButton(commitButton);
	tip->addButton(rePrintButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));
	connect(rePrintButton, SIGNAL(clicked()), this, SLOT(reprint()));

	Ui::IconButton *addPlateButton = new Ui::IconButton(":/res/fill-plate-24.png");
	addPlateButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	connect(addPlateButton, SIGNAL(clicked()), this, SLOT(addPlate()));

	QHBoxLayout *bLayout = new QHBoxLayout;
	bLayout->addWidget(_plateView);
	bLayout->addWidget(_detailView);
	bLayout->setStretch(0, 2);
	bLayout->setStretch(1, 1);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(addPlateButton, 0, 0);
	layout->addLayout(bLayout, 1, 0);
	layout->addWidget(tip, 0, 1, 2, 1);

	connect(_plateView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(showDetail(const QModelIndex &)));
}

void PackPanel::addPlate() {
	bool ok;
	QRegExp regExp("\\d{8,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入篮筐条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void PackPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Plate) {
		int id = bc.intValue();
		//if (!_plateView->hasPlate(id)) {
		_plateView->addPlate(id);
		//}
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		commit();
	}
}

void PackPanel::reprint() {
	RePrintDialog *d = new RePrintDialog(this);
	d->exec();
}

void PackPanel::commit() {
	QVariantList plates = _plateView->plates();
	if (plates.isEmpty()) {
		XNotifier::warn("请先添加需要打包的网篮");
		return;
	}

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	int checkerId = OperatorChooser::get(this, this);
	if (0 == checkerId) return;

	if (checkerId == opId)
	{
		XNotifier::warn("不允许使用同一个操作员进行审核");
		return;
	}

	_plateView->doPack(opId, checkerId);

	_detailView->clear();
}

void PackPanel::showDetail(const QModelIndex &index)
{
	int row = index.row();
	int column = index.column();
	if (index.parent().isValid())
	{
		QStandardItemModel *model = static_cast<QStandardItemModel*>(_plateView->model());
		QStandardItem *parentItem = model->itemFromIndex(model->index(0, 0));
		QString package_type_id = parentItem->child(row, 0)->data().toString();

		_detailView->loadDetail(package_type_id);
	}

}
