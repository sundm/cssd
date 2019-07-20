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
	if (testId.isEmpty()) return;

	Sterile::Result verdicts = _checkGroup->verdicts();
	if (verdicts.physics == itrac::NotChecked ||
		verdicts.chemistry == itrac::NotChecked) return;

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	QVariantMap vmap;
	vmap.insert("test_id", testId);
	vmap.insert("test_operator_id", opId);
	vmap.insert("phy_test_result", verdicts.toString(verdicts.physics));
	vmap.insert("che_test_result", verdicts.toString(verdicts.chemistry));
	QString bioResult = verdicts.toString(verdicts.bio);
	if (!bioResult.isEmpty()) vmap.insert("bio_test_result", bioResult);

	post(url(PATH_STERILE_CHECK), vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success())
			XNotifier::warn(QString("提交灭菌合格登记失败: ").append(resp.errorString()));
		else {
			XNotifier::warn("已完成灭菌合格登记");
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
			XNotifier::warn(QString("无法获取灭菌监测批次 [%1] 的数据: %2").arg(testId));
			return;
		}

		_testInfo->testId = testId;
		_testInfo->device = resp.getAsString("ste_device_name");
		_testInfo->cycle = resp.getAsInt("ste_cycle");
		_testInfo->startTimeStamp = std::move<QString>(resp.getAsString("ste_start_time")); // FIXME: redundant std::move?

		/*if ("1" == resp.getAsString("chemical_test")) {
			_result->chemistry = Sterile::Result::NotChecked;
		}
		if ("1" == resp.getAsString("biological_test")) {
			_result->bio = Sterile::Result::NotChecked;
		}

		QString result = resp.getAsString("result");
		if (result == "2")
			resultEdit->setText("待审核");
		else
			resultEdit->setText(QString("%1(已审核)").arg(result == "1" ? "合格" : "不合格"));*/

			// show info
		_infoGroup->updateInfo(*_testInfo);

		// show packages
		QList<QVariant> packages = resp.getAsList("packages");
		for(auto &package: packages) {
			QVariantMap map = package.toMap();
			_view->addPackage(map["package_id"].toString(), map["package_name"].toString());
		}
	});
}

void SterileExamPanel::reset() {
	_view->clear();
	_infoGroup->reset();
	_checkGroup->reset();
}
