#include "extdialog.h"
#include "xnotifier.h"
#include "core/constants.h"
#include "core/user.h"
#include "core/net/url.h"
#include "ui/groups.h"
#include "ui/inputfields.h"
#include "ui/buttons.h"
#include "widget/controls/combos.h"
#include "widget/controls/idedit.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QSpinBox>

ExtRecvDialog::ExtRecvDialog(QWidget *parent)
	: QDialog(parent)
	, _pkgEdit(new PackageEdit)
	, _countBox(new QSpinBox)
	, _vendorCombo(new VendorComboBox)
	, _genderCombo(new GenderComboBox)
	, _senderEdit(new Ui::FlatEdit)
	, _senderPhoneEdit(new Ui::FlatEdit)
	, _patientNameEdit(new Ui::FlatEdit)
	, _patientAgeEdit(new Ui::FlatEdit)
	, _patientIdEdit(new Ui::FlatEdit)
	, _doctorEdit(new Ui::FlatEdit)
{
	setWindowTitle("外来器械接收登记");

	_countBox->setValue(1);
	_senderPhoneEdit->setInputValidator(Ui::InputValitor::NumberOnly);
	_senderPhoneEdit->setMaxLength(Constant::maxPhoneNumber);
	_patientIdEdit->setInputValidator(Ui::InputValitor::LetterAndNumber);
	_patientAgeEdit->setInputValidator(Ui::InputValitor::NumberOnly);
	_patientAgeEdit->setMaxLength(3);

	FormGroup * pkgGroup = new FormGroup(this);
	pkgGroup->addRow("器械类型 (*)", _pkgEdit);
	pkgGroup->addRow("数量 (*)", _countBox);
	pkgGroup->addRow("供应商 (*)", _vendorCombo);

	FormGroup * transferGroup = new FormGroup(this);
	transferGroup->addRow("送货人 (*)", _senderEdit);
	transferGroup->addRow("联系电话 (*)", _senderPhoneEdit);

	FormGroup * patientGroup = new FormGroup(this);
	patientGroup->addRow("医生 (*)", _doctorEdit);
	patientGroup->addRow("患者姓名 (*)", _patientNameEdit);
	patientGroup->addRow("住院号", _patientIdEdit);
	patientGroup->addRow("年龄", _patientAgeEdit);
	patientGroup->addRow("性别", _genderCombo);
	
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("提交", Ui::BtnSize::Small);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(pkgGroup);
	layout->addWidget(transferGroup);
	layout->addWidget(patientGroup);
	layout->addWidget(commitButton);

	QTimer::singleShot(0, this, &ExtRecvDialog::loadData);
}

void ExtRecvDialog::accept() {
	int pkgId = _pkgEdit->currentId();
	int count = _countBox->value();
	int vendorId = _vendorCombo->currentData().toInt();

	QString sender = _senderEdit->text();
	QString senderPhone = _senderPhoneEdit->text();
	int receiverId = Core::currentUser().id;

	QString doctor = _doctorEdit->text();
	QString patient = _patientNameEdit->text();
	QString patientId = _patientIdEdit->text();
	QString patientAge = _patientAgeEdit->text();

	if (0 == pkgId || 0 == vendorId || sender.isEmpty() || senderPhone.isEmpty()
		|| doctor.isEmpty() || patient.isEmpty() || _genderCombo->currentIndex() == -1) {
		return;
	}

	// commit
	QVariantMap data;
	data.insert("pkg_type_id", pkgId);
	data.insert("num", count);
	data.insert("sup_id", vendorId);
	data.insert("send_name", sender);
	data.insert("send_phone", senderPhone);
	data.insert("p_operator_id", receiverId);
	data.insert("p_operator_name", "张春霞");
	data.insert("doctor_name", doctor);
	data.insert("patient_name", patient);
	data.insert("gender", _genderCombo->currentData());
	if (!patientId.isEmpty()) data.insert("admission", patientId);
	if (!patientAge.isEmpty()) data.insert("age", patientAge.toInt());

	data.insert("patient_name", patient);
	Url::post(Url::PATH_EXT_ADD, data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法提交登记: ").append(resp.errorString()));
			return;
		}
		QDialog::accept();
	});

}

void ExtRecvDialog::loadData() {
	_pkgEdit->loadForCategary("3");
	_vendorCombo->updateEntry();
}
