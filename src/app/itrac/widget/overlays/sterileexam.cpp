#include "sterileexam.h"
#include "tips.h"
#include "xnotifier.h"
#include "rdao/dao/flowdao.h"
#include "rdao/dao/PackageDao.h"
#include "rdao/entity/operator.h"
#include "rdao/entity/device.h"
#include "core/itracnamespace.h"
#include "core/datawrapper.h"
#include "core/net/url.h"
#include "core/user.h"
#include "ui/buttons.h"
#include "barcode.h"
#include "widget/controls/sterile_p.h"
#include "widget/controls/packageview.h"
#include "dialog/operatorchooser.h"
#include <QtWidgets/QtWidgets>

SterileExamPanel::SterileExamPanel(QWidget *parent)
	: CssdOverlayPanel(parent)
	, _infoGroup(new SterileInfoGroup)
	, _checkGroup(new SterileCheckGroup)
	, _view(new SterileCheckPackageView)
{
	connect(_infoGroup, &SterileInfoGroup::testIdChanged, this, &SterileExamPanel::updateSterileInfo);

	const QString text = "1 扫描灭菌监测条码\n2 对不同的监测类型进行审核，如有不合格项，填写不合格信息"
		"\n3 提交审核";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("提交灭菌审核");
	tip->addQr();
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_infoGroup, 0, 0);
	layout->addWidget(_checkGroup, 1, 0);

	layout->addWidget(_view, 0, 1, 2, 1);
	layout->addWidget(tip, 0, 2, 2, 1);
	layout->setColumnStretch(1, 1);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

}

void SterileExamPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Test) {
		updateSterileInfo(code);
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		commit();
	}
}

void SterileExamPanel::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	if (tc.type() == TranspondCode::Package)
	{
		updateSterileInfo(code);
	}

}

void SterileExamPanel::onBarcodeReceviced(const QString& code)
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

void SterileExamPanel::commit()
{
	SterilizeResult result;
	QString testId = _infoGroup->testId();
	if (testId.isEmpty()) {
		XNotifier::warn(QString("监测条码为空"));
		return;
	}
	
	Sterile::Result verdicts = _checkGroup->verdicts();
	if (_checkGroup->isFirst())
	{
		if (verdicts.physics == Rt::Unchecked) {
			XNotifier::warn(QString("物理检测结果为空"));
			return;
		}

		result.phyVerdict = verdicts.physics;

		if (verdicts.chemistry == Rt::Unchecked) {
			XNotifier::warn(QString("化学检测结果为空"));
			return;
		}

		result.cheVerdict = verdicts.chemistry;

		result.hasLabelOff = verdicts.lost;

		result.packages = _view->getPackages();
	}

	result.bioVerdict = verdicts.bio;
	if (result.phyVerdict == Rt::Unchecked && result.bioVerdict == Rt::Unchecked)
	{
		reset();
		return;
	}

	if (_needBio && verdicts.bio == Rt::Unchecked) {
		XNotifier::warn(QString("警告：该灭菌审核未提交生物检测，注意风险"));
	}
	Operator op;
	op.id = Core::currentUser().id;
	op.name = Core::currentUser().name;
	
	FlowDao dao;
	result_t resp = dao.updateSterilizationResult(testId, op, result);
	if (resp.isOk())
	{
		XNotifier::warn("已完成灭菌登记");
		reset();
	}
	else
	{
		XNotifier::warn(QString("提交灭菌登记失败: ").append(resp.msg()));
	}
}

void SterileExamPanel::updateSterileInfo(const QString &code) {
	PackageDao pkgDao;
	Package pkg;
	result_t resp = pkgDao.getPackage(code, &pkg);
	if (resp.isOk())
	{
		reset();

		FlowDao dao;
		DeviceBatchInfo info;

		resp = dao.getDeviceBatchInfoByPackage(pkg, &info);

		if (resp.isOk())
		{
			//set info
			_infoGroup->updateInfo(info);
			_checkGroup->updateInfo(info.result);

			// show packages
			bool readOnly = info.result.phyVerdict != Rt::SterilizeVerdict::Unchecked;
			_view->addPackages(info.result.packages, readOnly);
		}
		else
		{
			XNotifier::warn(QString("无法获取包[%1]灭菌信息: %2").arg(code).arg(resp.msg()));
			return;
		}
	}
	else
	{
		XNotifier::warn(QString("无法获取包[%1]信息: %2").arg(code).arg(resp.msg()));
		return;
	}
}

void SterileExamPanel::reset() {
	_view->clear();
	_infoGroup->reset();
	_checkGroup->reset();
}
