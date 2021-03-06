#include "dispatch.h"
#include "tips.h"
#include "xnotifier.h"
#include "barcode.h"

#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/inputfields.h"
#include "widget/controls/idedit.h"
#include "widget/controls/packageview.h"
#include "dialog/operatorchooser.h"

#include <QtWidgets/QtWidgets>


OrDispatchPanel::OrDispatchPanel(QWidget *parent)
	: CssdOverlayPanel(parent)
	, _deptEdit(new DeptEdit)
{
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);

	hLayout->addWidget(new QLabel("目标科室"));
	hLayout->addWidget(_deptEdit);

	_pkgView = new DispatchPackageView;

	const QString text = "1 选择或扫描物品包发放的目标科室\n2 扫描或输入物品包条码\n3确认发放";

	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("确认发放");
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(hLayout, 0, 0);
	layout->addWidget(_pkgView, 1, 0);
	layout->addWidget(tip, 0, 1, 2, 1);

	QTimer::singleShot(500, [this] { _deptEdit->load(); });
}

void OrDispatchPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Package && !_pkgView->hasPackage(code)) {
		_pkgView->addPackage(code);
	}
}

void OrDispatchPanel::commit() {
	int deptId = _deptEdit->currentId();
	if (0 == deptId) return;

	QVariantList packages = _pkgView->packages();
	if (packages.isEmpty()) return;

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	QVariantMap vmap;
	vmap.insert("department_id", deptId);
	vmap.insert("operator_id", opId);
	vmap.insert("package_ids", packages);

	Url::post(Url::PATH_ISSUE_ADD, vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("提交发放失败: ").append(resp.errorString()));
		}
		else {
			XNotifier::warn("发放成功!");
			reset();
		}
	});
}

void OrDispatchPanel::reset() {
	_pkgView->clear();
	_deptEdit->reset();
}
