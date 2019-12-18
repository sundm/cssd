#include "check.h"
#include "tips.h"
#include "core/constants.h"
#include "core/barcode.h"
#include "core/assets.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "core/user.h"
#include "inliner.h"
#include "widget/controls/packageview.h"
#include "widget/controls/idedit.h"
#include "dialog/operatorchooser.h"
#include "dialog/regexpinputdialog.h"
#include "importextdialog.h"
#include "model/itemdelegate.h"
#include "xnotifier.h"
#include "rdao/dao/surgerydao.h"
#include "rdao/dao/flowdao.h"
#include "rdao/dao/PackageDao.h"
#include "rdao/entity/operator.h"
#include "rdao/dao/flowDao.h"
#include "rdao/dao/instrumentdao.h"
#include <xui/images.h>
#include <xui/imageviewer.h>
#include <QtWidgets/QtWidgets>

PreBindPanel::PreBindPanel(QWidget *parent /*= nullptr*/)
	: CssdOverlayPanel(parent)
	, _operInfoView(new OperationInfoView)
	, _operPackageView(new OperationPackageView)
	, _unusualCodes(new QStringList)
	, _scannedCodes(new QStringList)
{
	//const QString text = "1 扫描物品包ID\n2 扫描托盘内器械\n3 确认回收 \n说明\n灰色：实际数量\n绿色：通过数量\n黄色：剩余数量\n红色：异常数量";
	Tip *tip = new Tip();
	
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("完成绑定");
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));
	Ui::PrimaryButton *resetButton = new Ui::PrimaryButton("重置操作");
	connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

	tip->addQr();
	tip->addButton(commitButton);
	tip->addButton(resetButton);

	QVBoxLayout *aLayout = new QVBoxLayout;
	aLayout->addWidget(_operInfoView);
	aLayout->addWidget(_operPackageView);
	aLayout->setStretch(1, 1);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addLayout(aLayout);
	layout->addWidget(tip);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	connect(_operInfoView, SIGNAL(operation(const int)), this, SLOT(loadPackage(const int)));
	connect(_operPackageView, SIGNAL(packageClicked(const Package&)), this, SLOT(loadInsturment(const Package&)));

	initOperationView();
}

void PreBindPanel::loadPackage(const int surgeryId)
{
	_operPackageView->loadPackages(surgeryId);
}

void PreBindPanel::initOperationView() {
	_operInfoView->loadSurgeries();
}

void PreBindPanel::onScanned(const QString& code)
{
	if (!_scannedCodes->contains(code))
	{
		_scannedCodes->append(code);
	}
}

void PreBindPanel::onUnusual(const QString& code)
{
	if (!_unusualCodes->contains(code))
	{
		_unusualCodes->append(code);
	}
}

void PreBindPanel::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	if (tc.type() == TranspondCode::Package)
	{
		PackageDao dao;
		Package pkg;
		result_t resp = dao.getPackage(code, &pkg, true);
		if (resp.isOk())
		{
			if (pkg.status != Rt::FlowStatus::Dispatched)
			{
				XNotifier::warn(QString("该包尚未完成发放，请先进行发放登记"));
				return;
			}

			PackageQualityControl pkgqc;
			resp = dao.getPackageQualityControl(pkg, &pkgqc);
			if (resp.isOk())
			{
				if (pkgqc.bioResult == Rt::SterilizeVerdict::Unqualified)
				{
					XNotifier::warn(QString("该包生物监测结果不合格，不能进行绑定操作。"));
					return;
				}

				if (pkgqc.isExpired)
				{
					XNotifier::warn(QString("该包已失效，不能进行绑定操作。"));
					return;
				}

				if (pkgqc.isRecalled)
				{
					XNotifier::warn(QString("该包已被召回，不能进行绑定操作。"));
					return;
				}
			}
			else
			{
				XNotifier::warn(QString("无法获取包质控信息: ").append(resp.msg()));
				return;
			}

			_operPackageView->addPackage(pkg);
		}
		else
		{
			XNotifier::warn(QString("获取包信息失败: ").append(resp.msg()));
		}

	}
}

void PreBindPanel::onBarcodeReceviced(const QString& code)
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

void PreBindPanel::handleBarcode(const QString &code) {
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

void PreBindPanel::commit() {
	//todo
	
	if (_operPackageView->isFinished())
	{
		Surgery surgery = _operPackageView->getSurgery();

		Operator op;
		op.id = Core::currentUser().id;
		op.name = Core::currentUser().name;

		FlowDao dao;
		result_t resp = dao.addSurgeryBindPackages(surgery, op);

		if (resp.isOk())
		{
			XNotifier::warn(QString("出库成功！"));
			reset();
		}
		else
		{
			XNotifier::warn(QString("出库失败: ").append(resp.msg()));
		}
	}
	else
	{
		XNotifier::warn(QString("尚未完成绑定，不能提交！"));
	}
	
}

void PreBindPanel::reset()
{
	_operInfoView->refresh();
	_operPackageView->clear();
}

OperExamPanel::OperExamPanel(Rt::SurgeryStatus status, QWidget *parent /*= nullptr*/)
	: CssdOverlayPanel(parent)
	, _operInfoView(new OperationInfoTabelView)
	, _operPackageView(new OperationCheckPackageView)
	, _pkgView(new PackageSimpleInfoView)
	, _detailView(new PackageDetailView)
	, _unusualView(new UnusualInstrumentView)
	, _unusualCodes(new QStringList)
	, _scannedCodes(new QStringList)
{
	//const QString text = "1 扫描物品包ID\n2 扫描托盘内器械\n3 确认回收 \n说明\n灰色：实际数量\n绿色：通过数量\n黄色：剩余数量\n红色：异常数量";
	//Tip *tip = new Tip(text);

	//Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("完成检查");
	//connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));
	//Ui::PrimaryButton *resetButton = new Ui::PrimaryButton("重置操作");
	//connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));

	//tip->addQr();
	//tip->addButton(commitButton);
	//tip->addButton(resetButton);

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
	//layout->addWidget(tip);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));
	
	connect(_operInfoView, SIGNAL(operationClicked(const int)), this, SLOT(loadPackage(const int)));
	connect(_detailView, SIGNAL(scand(const QString&)), this, SLOT(onScanned(const QString&)));
	connect(_detailView, SIGNAL(unusual(const QString&)), this, SLOT(onUnusual(const QString&)));
	connect(_operPackageView, SIGNAL(waitForScan(const QList<Package> &)), this, SLOT(loadInsturments(const QList<Package> &)));

	_step = -1;
	_status = status;
	initOperationView();
}

void OperExamPanel::loadPackage(const int surgeryId)
{
	_step = 0;
	_operPackageView->loadPackages(surgeryId);
}

void OperExamPanel::loadInsturments(const QList<Package>& pkgs)
{
	QList<Instrument> insList;
	_packages.clear();
	_packages.append(pkgs);

	for each (Package pkg in _packages)
	{
		insList.append(pkg.instruments);
	}

	_pkgView->updatePackageInfo(insList.count());
	_detailView->loadDetail(&insList);
	_step = 1;
}

void OperExamPanel::initOperationView() {
	_operInfoView->loadSurgeries(_status);
}

void OperExamPanel::onScanned(const QString& code)
{
	if (!_scannedCodes->contains(code))
	{
		_scannedCodes->append(code);
		_pkgView->scanned();

		for each (Package pkg in _packages)
		{
			for each (Instrument ins in pkg.instruments)
			{
				if (code.compare(ins.udi) == 0)
				{
					_operPackageView->setScanned(pkg.udi);
				}
			}
		}
	}
}

void OperExamPanel::onUnusual(const QString& code)
{
	if (!_unusualCodes->contains(code))
	{
		_unusualCodes->append(code);
		_pkgView->unusualed();
		_unusualView->addUnusual(code);
	}
}

void OperExamPanel::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);

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

void OperExamPanel::onBarcodeReceviced(const QString& code)
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

void OperExamPanel::handleBarcode(const QString &code) {
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

void OperExamPanel::commit() {
	if (_pkgView->isScanFinished())
	{
		Surgery surgery = _operPackageView->getSurgery();

		Operator op;
		op.id = Core::currentUser().id;
		op.name = Core::currentUser().name;

		FlowDao dao;
		result_t resp;
		if (_status == Rt::SurgeryStatus::UdiPackageBound)
		{
			resp = dao.addSurgeryPreCheck(surgery.id, op);

			if (resp.isOk())
			{
				XNotifier::warn(QString("术前检查登记成功"));
				reset();
			}
			else
			{
				XNotifier::warn(QString("术前检查登记失败: ").append(resp.msg()));
			}
		}

		if (_status == Rt::SurgeryStatus::PreChecked)
		{
			resp = dao.addSurgeryPostCheck(surgery.id, op);

			if (resp.isOk())
			{
				XNotifier::warn(QString("术后检查登记成功"));
				reset();
			}
			else
			{
				XNotifier::warn(QString("术后检查登记失败: ").append(resp.msg()));
			}
		}
		
		
	}
	else
	{
		XNotifier::warn(QString("尚未完成绑定，不能提交！"));
	}
}

void OperExamPanel::reset()
{
	//todo
	_scannedCodes->clear();
	_unusualCodes->clear();
	_operInfoView->loadSurgeries(_status);
	_operPackageView->clear();
	_pkgView->reset();
	_detailView->clear();
	_unusualView->clear();
	_step = 0;
}