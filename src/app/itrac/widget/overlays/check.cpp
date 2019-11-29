#include "check.h"
#include "tips.h"
#include "core/constants.h"
#include "core/barcode.h"
#include "core/assets.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "inliner.h"
#include "widget/controls/packageview.h"
#include "widget/controls/idedit.h"
#include "dialog/operatorchooser.h"
#include "dialog/regexpinputdialog.h"
#include "importextdialog.h"
#include "model/spinboxdelegate.h"
#include "xnotifier.h"
#include <xui/images.h>
#include <xui/imageviewer.h>
#include <QtWidgets/QtWidgets>

PreExamPanel::PreExamPanel(QWidget *parent /*= nullptr*/)
	: CssdOverlayPanel(parent)
	, _operInfoView(new OperationInfoView)
	, _operPackageView(new OperationPackageView)
	, _pkgView(new PackageSimpleInfoView)
	, _detailView(new PackageDetailView)
	, _unusualView(new UnusualInstrumentView)
	, _codeMap(new QHash<QString, QString>)
	, _unusualCodes(new QStringList)
	, _scannedCodes(new QStringList)
{
	const QString text = "1 扫描物品包ID\n2 扫描托盘内器械\n3 确认回收 \n说明\n灰色：实际数量\n绿色：通过数量\n黄色：剩余数量\n红色：异常数量";
	Tip *tip = new Tip(text);

	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("完成检查");
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));
	Ui::PrimaryButton *resetButton = new Ui::PrimaryButton("重置操作");
	connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

	tip->addQr();
	tip->addButton(resetButton);
	tip->addButton(commitButton);

	QVBoxLayout *aLayout = new QVBoxLayout;
	aLayout->addWidget(_operInfoView);
	aLayout->addWidget(_operPackageView);
	aLayout->setStretch(1, 1);

	QVBoxLayout *bLayout = new QVBoxLayout;
	bLayout->addWidget(_pkgView);
	bLayout->addWidget(_detailView);
	bLayout->addWidget(_unusualView);
	bLayout->setStretch(1, 1);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addLayout(aLayout);
	layout->addLayout(bLayout);
	layout->addWidget(tip);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	_step = 0;
}

void PreExamPanel::initOperationView() {

}

void PreExamPanel::onTransponderReceviced(const QString& code)
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

		_pkgView->updatePackageInfo(_codeMap->size());
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

void PreExamPanel::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}

void PreExamPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if ((bc.type() == Barcode::Package || bc.type() == Barcode::PkgCode)) {//&& !_pkgView->hasPackage(code)) {
		//_pkgView->addPackage(code);
	}
	else if (bc.type() == Barcode::Plate) {
		//updatePlate(code);
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		commit();
	}
}

void PreExamPanel::commit() {
	//todo
}

void PreExamPanel::reset()
{
	//todo
}


PostExamPanel::PostExamPanel(QWidget *parent /*= nullptr*/)
	: CssdOverlayPanel(parent)
	, _operInfoTableView(new OperationInfoTabelView)
	, _operPackageView(new OperationPackageView)
	, _pkgView(new PackageSimpleInfoView)
	, _detailView(new PackageDetailView)
	, _unusualView(new UnusualInstrumentView)
	, _codeMap(new QHash<QString, QString>)
	, _unusualCodes(new QStringList)
	, _scannedCodes(new QStringList)
{
	const QString text = "1 扫描物品包ID\n2 扫描托盘内器械\n3 确认回收 \n说明\n灰色：实际数量\n绿色：通过数量\n黄色：剩余数量\n红色：异常数量";
	Tip *tip = new Tip(text);
	
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("完成检查");
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));
	Ui::PrimaryButton *resetButton = new Ui::PrimaryButton("重置操作");
	connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

	tip->addQr();
	tip->addButton(resetButton);
	tip->addButton(commitButton);
	
	QVBoxLayout *aLayout = new QVBoxLayout;
	aLayout->addWidget(_operInfoTableView);
	aLayout->addWidget(_operPackageView);
	aLayout->setStretch(1, 1);

	QVBoxLayout *bLayout = new QVBoxLayout;
	bLayout->addWidget(_pkgView);
	bLayout->addWidget(_detailView);
	bLayout->addWidget(_unusualView);
	bLayout->setStretch(1, 1);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addLayout(aLayout);
	layout->addLayout(bLayout);
	layout->addWidget(tip);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	_step = 0;
}

void PostExamPanel::initOperationView() {

}

void PostExamPanel::onTransponderReceviced(const QString& code)
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

		_pkgView->updatePackageInfo(_codeMap->size());
		_detailView->loadDetail(_codeMap);*/

		_step = 1;
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

void PostExamPanel::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}

void PostExamPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if ((bc.type() == Barcode::Package || bc.type() == Barcode::PkgCode)) {//&& !_pkgView->hasPackage(code)) {
		//_pkgView->addPackage(code);
	}
	else if (bc.type() == Barcode::Plate) {
		//updatePlate(code);
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		commit();
	}
}

void PostExamPanel::commit() {
	//todo
}

void PostExamPanel::reset()
{
	//todo
}
