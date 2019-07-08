#include "usepanel.h"
#include "tips.h"

#include "core/constants.h"
#include "core/barcode.h"
#include "core/assets.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "widget/controls/packageview.h"
#include "widget/controls/idedit.h"

#include <QtWidgets/QtWidgets>


UsePanel::UsePanel(QWidget *parent /*= nullptr*/)
	: Ui::Source(parent)
	, Scanable()
	, _pkgView(new OrRecyclePackageView)
{
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "手工添加");
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *minusButton = new Ui::IconButton(":/res/delete-24.png", "删除选中");
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	hLayout->addStretch();

	const QString text = "1扫描患者所使用的物品包 \n2 扫描患者腕带上的条码\n3 确认使用";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("确认使用");
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(hLayout, 0, 0);
	layout->addWidget(_pkgView, 1, 0);
	layout->addWidget(tip, 1, 1);
}

void UsePanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Package && !_pkgView->hasPackage(code)) {
		_pkgView->addPackage(code);
	}
	else{
		updatePatient(code);
	}
}

void UsePanel::addEntry() {
	bool ok;
	QRegExp regExp("\\d{10,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入包上的条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void UsePanel::removeEntry() {

}

void UsePanel::commit() {
	QVariantList packages = _pkgView->packages();
	if (packages.isEmpty()) return;

	QString patientId = "12345678";

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	QVariantList pkgList;
	for (int i = 0; i != packages.size(); i++) {
		QVariantMap package;
		package.insert("package_id", packages[i]);
		package.insert("recycle_reason", 1);
		pkgList << package;
	}

	QVariantMap vmap;
	vmap.insert("packages", pkgList);
	vmap.insert("plate_id", patientId);
	vmap.insert("operator_id", opId);

	Url::post(Url::PATH_RECYCLE_ADD, vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("回收登记失败: ").append(resp.errorString()));
		}
		else {
			XNotifier::warn("回收成功!");
			reset();
		}
	});
}

void UsePanel::updatePatient(const QString &plateId) {
	_pkgView->updatePlate(plateId);
}

void UsePanel::reset() {
	_pkgView->clear();
}
