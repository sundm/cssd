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
#include "dialog/regexpinputdialog.h"
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

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "手工添加");
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *minusButton = new Ui::IconButton(":/res/delete-24.png", "删除选中");
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	hLayout->addStretch();

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(hLayout, 0, 0);
	layout->addWidget(_pkgView, 1, 0);
	layout->addWidget(tip, 0, 1, 2, 1);

	QTimer::singleShot(500, [this] { _deptEdit->load(DeptEdit::OPERATING_ROOM); });
}

void OrDispatchPanel::addEntry() {
	bool ok;
	QRegExp regExp("\\d{10,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入包条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void OrDispatchPanel::removeEntry() {
	QItemSelectionModel *selModel = _pkgView->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_pkgView->model()->removeRow(indexes.at(i - 1).row());
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

	post(url(PATH_ISSUE_ADD), vmap, [=](QNetworkReply *reply) {
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
