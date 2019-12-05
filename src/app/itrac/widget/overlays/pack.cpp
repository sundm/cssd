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
#include "core/user.h"
#include "xnotifier.h"
#include "rdao/dao/PackageDao.h"
#include "rdao/dao/instrumentdao.h"
#include "rdao/entity/operator.h"
#include "rdao/dao/flowdao.h"
#include <xui/images.h>
#include <xui/imageviewer.h>
#include "util/printermanager.h"
#include <printer/labelprinter.h>
#include <QtWidgets/QtWidgets>

PackPanel::PackPanel(QWidget *parent) : CssdOverlayPanel(parent) 
	, _pkgView(new PackageInfoView)
	, _detailView(new PackageDetailView)
	, _unusualView(new UnusualInstrumentView)
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
	Ui::PrimaryButton *resetButton = new Ui::PrimaryButton("重置");
	tip->addQr();
	tip->addButton(commitButton);
	tip->addButton(rePrintButton);
	tip->addButton(abnormalButton);
	tip->addButton(resetButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));
	connect(rePrintButton, SIGNAL(clicked()), this, SLOT(reprint()));
	connect(abnormalButton, SIGNAL(clicked()), this, SLOT(abnormal()));
	connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

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

	connect(_detailView, SIGNAL(scand(const QString&)), this, SLOT(onScanned(const QString&)));
	connect(_detailView, SIGNAL(unusual(const QString&)), this, SLOT(onUnusual(const QString&)));
	connect(_detailView, SIGNAL(onclick(const QString&)), this, SLOT(loadInstrumentImg(const QString&)));

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

void PackPanel::commit() 
{
	if (!_pkgView->isScanFinished())
	{
		XNotifier::warn("尚未完成器械检查，或存在异常器械");
		return;
	}

	Operator op_o, op_a;
	op_o.id = Core::currentUser().id;
	op_o.name = Core::currentUser().name;
	op_a.id = 110002;
	op_a.name = "张爱琴";

	FlowDao dao;
	LabelInfo linfo;

	result_t resp = dao.addPack(_package, op_o, op_a, &linfo);
	if (resp.isOk())
	{
		reset();

		LabelPrinter *printer = PrinterManager::currentPrinter();
		if (0 != printer->open(LABEL_PRINTER)) {
			XNotifier::warn("打印机未就绪");
			return;
		}

		PackageLabel pLabel;

		pLabel.operatorName = op_o.name;
		pLabel.assessorName = op_a.name;
		pLabel.packageId = linfo.labelId;
		pLabel.packageName = _package.name;
		pLabel.packageFrom = _package.dept.name;
		pLabel.packageType = _package.packType.name;
		pLabel.disinDate = linfo.packDate.toString("yyyy-MM-dd");
		pLabel.expiryDate = linfo.expireDate.toString("yyyy-MM-dd");

		pLabel.count = _package.instruments.size();
		printer->printPackageLabel(pLabel);

		printer->close();
	}
	else
	{
		XNotifier::warn(QString("配包失败: ").append(resp.msg()));
		return;
	}
	
}

void PackPanel::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	if (tc.type() == TranspondCode::Package && 0 == _step)
	{
		PackageDao dao;
		result_t resp = dao.getPackage(code, &_package, true);
		if (resp.isOk())
		{
			/*if (_package.status == Rt::Recycled) {
				XNotifier::warn("该包已回收，请勿重复回收");
				return;
			}*/
			_unusualCodes->clear();
			_scannedCodes->clear();

			_pkgView->updatePackageInfo(code, _package.name, _package.instruments.size());
			_detailView->loadDetail(&_package.instruments);
			loadPackageImg(code);
			_step = 1;
		}

		_step = 1;
	}

	if (tc.type() == TranspondCode::Instrument && 1 == _step)
	{
		InstrumentDao dao;
		Instrument it;
		result_t resp = dao.getInstrument(code, &it);
		if (resp.isOk())
		{
			_detailView->scanned(code);
		}
	}
}

void PackPanel::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
	Barcode bc(code);
	if (bc.type() == Barcode::Commit) {
		commit();
	}

	if (bc.type() == Barcode::Reset) {
		reset();
	}
}

void PackPanel::onScanned(const QString& code)
{
	if (!_scannedCodes->contains(code))
	{
		_scannedCodes->append(code);
		_pkgView->scanned();
	}
}

void PackPanel::onUnusual(const QString& code)
{
	if (!_unusualCodes->contains(code))
	{
		_unusualCodes->append(code);
		_pkgView->unusualed();
		_unusualView->addUnusual(code);
	}
}

void PackPanel::reset()
{
	_pkgView->reset();
	_detailView->reset();
	_unusualView->reset();

	_scannedCodes->clear();
	_unusualCodes->clear();
	_step = 0;

	QString fileName = QString("./photo/timg.png");
	_pkgImg->setImage(fileName);
	_insImg->setImage(fileName);
}

void PackPanel::loadPackageImg(const QString& udi)
{
	QString imgPath = QString("./photo/package/%1.png").arg(udi);
	QFile file(imgPath);
	if (file.exists()) {
		_pkgImg->setImage(imgPath);
	}
	else {
		QString fileName = QString("./photo/timg.png");
		_pkgImg->setImage(fileName);
	}
}

void PackPanel::loadInstrumentImg(const QString& udi)
{
	QString imgPath = QString("./photo/instrument/%1.png").arg(udi);
	QFile file(imgPath);
	if (file.exists()) {
		_insImg->setImage(imgPath);
	}
	else
	{
		QString fileName = QString("./photo/timg.png");
		_insImg->setImage(fileName);
	}
}