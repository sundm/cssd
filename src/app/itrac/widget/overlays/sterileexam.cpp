#include "sterileexam.h"
#include "tips.h"
#include "xnotifier.h"

#include "core/itracnamespace.h"
#include "core/datawrapper.h"
#include "core/net/url.h"
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
	, _testInfo(new Sterile::TestInfo)
	, _resultInfo(new Sterile::Result)
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

void SterileExamPanel::commit()
{
	QString testId = _infoGroup->testId();
	if (testId.isEmpty()) {
		XNotifier::warn(QString("监测条码为空"));
		return;
	}

	Sterile::Result verdicts = _checkGroup->verdicts();
	if (verdicts.physics == itrac::NotChecked ) {
		XNotifier::warn(QString("物理检测结果为空"));
		return;
	}

	if (verdicts.chemistry == itrac::NotChecked) {
		XNotifier::warn(QString("化学检测结果为空"));
		return;
	}

	if (_needBio && verdicts.bio == itrac::NotChecked) {
		XNotifier::warn(QString("警告：该灭菌审核未提交生物检测，注意风险"));
	}

	QVariantMap vmap;
	vmap.insert("test_id", testId);
	
	
	if (verdicts.physics != -1)
		vmap.insert("phy_test_result", verdicts.toString(verdicts.physics));

	if (verdicts.chemistry != -1)
		vmap.insert("che_test_result", verdicts.toString(verdicts.chemistry));
	
	if (verdicts.bio != -1 && verdicts.bio != 2)
		vmap.insert("bio_test_result", verdicts.toString(verdicts.bio));

	if (verdicts.wet != -1)
		vmap.insert("outside_result", verdicts.toString(verdicts.wet));

	if (verdicts.lost != -1)
		vmap.insert("label_off", verdicts.toString(verdicts.lost));

	if (vmap.size() == 1)
	{
		XNotifier::warn("灭菌登记未发生变动");
		reset();
		return;
	}

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;
	vmap.insert("test_operator_id", opId);

	post(url(PATH_STERILE_CHECK), vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success())
			XNotifier::warn(QString("提交灭菌登记失败: ").append(resp.errorString()));
		else {
			XNotifier::warn("已完成灭菌登记");
			reset();
		}
	});
}

void SterileExamPanel::updateSterileInfo(const QString &testId) {
	reset();

	QByteArray data("{\"test_id\":");
	data.append(testId).append('}');
	post(url(PATH_STERILE_INFO), data, [this, testId](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取灭菌监测批次 [%1] 的数据: %2").arg(testId).arg(resp.errorString()));
			return;
		}

		_testInfo->testId = testId;
		_testInfo->device = resp.getAsString("ste_device_name");
		_testInfo->cycle = resp.getAsInt("ste_cycle");
		_testInfo->startTimeStamp = std::move<QString>(resp.getAsString("ste_start_time")); // FIXME: redundant std::move?

		_needBio = false;
		if ("1" == resp.getAsString("biological_test")) {
			_needBio = true;
		}

		// show info
		_infoGroup->updateInfo(*_testInfo);

		QList<QVariant> results = resp.getAsList("test_result");
		_resultInfo->physics = results.at(0).toInt();
		_resultInfo->bio = results.at(1).toInt();
		_resultInfo->chemistry = results.at(2).toInt();
		_resultInfo->wet = results.at(3).toInt();
		_resultInfo->lost = results.at(4).toInt();

		_checkGroup->updateInfo(*_resultInfo);

		// show packages
		QList<QVariant> packages = resp.getAsList("packages");
		for(auto &package: packages) {
			QVariantMap map = package.toMap();
			_view->addPackage(map["package_id"].toString(), map["package_name"].toString(), map["implant"].toBool());
		}
	});
}

void SterileExamPanel::reset() {
	_view->clear();
	_infoGroup->reset();
	_checkGroup->reset();
}
