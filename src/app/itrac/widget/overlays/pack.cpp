#include "pack.h"
#include "barcode.h"
#include "devicewidget.h"
#include "tips.h"
#include "ui/buttons.h"
#include "inliner.h"
#include "widget/controls/plateview.h"
#include "controls/packageview.h"
#include "dialog/regexpinputdialog.h"
#include "dialog/operatorchooser.h"
#include "dialog/reprintdialog.h"
#include "dialog/washabnormal.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include <xui/images.h>
#include <xui/imageviewer.h>
#include <printer/labelprinter.h>
#include <QtWidgets/QtWidgets>

PackPanel::PackPanel(QWidget *parent) : CssdOverlayPanel(parent) 
	, _pkgView(new PackageInfoView)
	, _detailView(new PackageDetailView)
	, _unusualView(new UnusualInstrumentView)
	, _codeMap(new QHash<QString, QString>)
	, _unusualCodes(new QStringList)
	, _scannedCodes(new QStringList)
	, _pkgImg(new XPicture(this))
	, _insImg(new XPicture(this))
{
	//connect(_plateView, &PackPlateView::packed, this, &PackPanel::print);

	const QString text = "1 扫描或输入托盘条码\n2 开始配包"
		"\n\n注意：请根据当前标签打印机选择对应的物品托盘";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("开始配包");
	Ui::PrimaryButton *rePrintButton = new Ui::PrimaryButton("重新打印");
	Ui::PrimaryButton *abnormalButton = new Ui::PrimaryButton("异常登记");
	tip->addQr();
	tip->addButton(commitButton);
	tip->addButton(rePrintButton);
	tip->addButton(abnormalButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));
	connect(rePrintButton, SIGNAL(clicked()), this, SLOT(reprint()));
	connect(abnormalButton, SIGNAL(clicked()), this, SLOT(abnormal()));

	QVBoxLayout *aLayout = new QVBoxLayout;
	aLayout->addWidget(_pkgView);
	aLayout->addWidget(_detailView);
	aLayout->addWidget(_unusualView);
	aLayout->setStretch(1, 1);

	QString fileName = QString("./photo/timg.png");
	_pkgImg->setBgColor(QColor(245, 246, 247));
	_pkgImg->setImage(fileName);
	_pkgImg->setMinimumWidth(300);
	_insImg->setBgColor(QColor(245, 246, 247));
	_insImg->setImage(fileName);
	_insImg->setMinimumWidth(300);

	QVBoxLayout *imgLayout = new QVBoxLayout;
	imgLayout->addWidget(_pkgImg);
	imgLayout->addWidget(_insImg);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addLayout(aLayout);
	layout->addLayout(imgLayout);
	layout->addWidget(tip);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	_step = 0;
}

void PackPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
}

void PackPanel::reprint() {
	RePrintDialog *d = new RePrintDialog(this);
	d->exec();
}

void PackPanel::abnormal() {
	WashAbnormal *d = new WashAbnormal(this);
	d->exec();
}

void PackPanel::commit() {

}

void PackPanel::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	if (tc.type() == TranspondCode::Package && 0 == _step)
	{
		/*_codeMap->clear();
		_unusualCodes->clear();
		_scannedCodes->clear();
		_codeMap->insert("E2009A8020020AF000006502", "测试器械01");
		_codeMap->insert("E2009A8020020AF000005618", "测试器械01");
		_codeMap->insert("E2009A8020020AF000006342", "测试器械01");

		_codeMap->insert("E2009A8020020AF000006090", "测试器械02");
		_codeMap->insert("E2009A8020020AF000004398", "测试器械02");
		_codeMap->insert("E2009A8020020AF000006048", "测试器械02");
		_codeMap->insert("E2009A8020020AF000003250", "测试器械02");
		_codeMap->insert("E2009A8020020AF000005811", "测试器械02");

		_codeMap->insert("E2009A8020020AF000005187", "测试器械03");

		_pkgView->updatePackageInfo(code, QString("RFID测试器械包001"), _codeMap->size());
		_detailView->loadDetail(_codeMap);

		_step = 1;*/
	}

	if (tc.type() == TranspondCode::Instrument && 1 == _step)
	{
		if (_codeMap->size() > 0 && _codeMap->contains(code) && !_scannedCodes->contains(code))
		{
			_scannedCodes->append(code);
			_pkgView->scanned();
			_detailView->scanned(code);
		}
		else if (_codeMap->size() > 0 && !_codeMap->contains(code))
		{
			if (!_unusualCodes->contains(code))
			{
				_unusualCodes->append(code);
				_pkgView->unusualed();
				_unusualView->addUnusual(code);
			}

		}
	}
}

void PackPanel::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}

void PackPanel::reset()
{
	//todo
}