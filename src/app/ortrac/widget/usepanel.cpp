#include "usepanel.h"
#include "core/constants.h"
#include "core/barcode.h"
#include "core/user.h"
#include "core/net/url.h"
#include "dialog/operatorchooser.h"
#include "dialog/regexpinputdialog.h"
#include "xnotifier.h"

UsePanel::UsePanel(QWidget *parent)
	: Ui::Source(parent), Scanable()
{
	setupUi(this);
	connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));
	connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
	connect(bcInputBtn, SIGNAL(clicked()), this, SLOT(addEntry()));
	connect(removeBtn, SIGNAL(clicked()), this, SLOT(remove()));
}

UsePanel::~UsePanel()
{
}

void UsePanel::addEntry()
{
	bool ok;
	QRegExp regExp("\\d{10,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入包上的条码", "", regExp, &ok);
	if (ok) {
		Barcode bc(code);
		if (bc.type() == Barcode::Package)
			handleBarcode(code);
		else
			XNotifier::warn(QString("请输入包条码"));
	}
}

void UsePanel::remove()
{
	QItemSelectionModel *selModel = pkgView->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		pkgView->model()->removeRow(indexes.at(i - 1).row());
}

void UsePanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Package) {
		if (!pkgView->hasPackage(code))
			pkgView->addPackage(code);
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		submit();
	}
	else {
		idEdit->setText(code);
		idEdit->setFocus();
	}
}

void UsePanel::submit()
{
	QString patientId = idEdit->text();
	if (patientId.isEmpty()) return;
	QString room = roomEdit->text();
	QString deskNo = deskEdit->text();

	QVariantList packages = pkgView->packages();
	if (packages.isEmpty()) return;

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	Core::User &user = Core::currentUser();

	QVariantMap vmap;
	vmap.insert("package_ids", packages);
	vmap.insert("patient_id", patientId);
	vmap.insert("department_id", user.deptId);
	vmap.insert("nurse_id", opId);
	if (room.length()) vmap.insert("operator_room", room);
	if (deskNo.length()) vmap.insert("operator_table", deskNo.toInt());

	post(url(PATH_USE_ADD), vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("登记失败: ").append(resp.errorString()));
		}
		else {
			XNotifier::warn("登记成功!");
			reset();
		}
	});
}

void UsePanel::reset() {
	idEdit->clear();
	roomEdit->clear();
	deskEdit->clear();
	pkgView->clear();
}
