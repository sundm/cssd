#include "addextdialog.h"
#include "xnotifier.h"
#include "core/constants.h"
#include "core/user.h"
#include "core/net/url.h"
#include "ui/groups.h"
#include "ui/inputfields.h"
#include "ui/buttons.h"
#include "widget/controls/combos.h"
#include "widget/controls/idedit.h"
#include "ui/views.h"
#include <QtWidgets/QtWidgets>
#include <QDateTime>
#include <QTimer>
#include <QSpinBox>

AddExtDialog::AddExtDialog(QWidget *parent, QStandardItemModel *model, int row)
	: QDialog(parent)
	, _pkgEdit(new PackageEdit)
	, _countBox(new QSpinBox)
	, _genderCombo(new GenderComboBox)
	, _patientNameEdit(new Ui::FlatEdit)
	, _patientAgeEdit(new Ui::FlatEdit)
	, _patientIdEdit(new Ui::FlatEdit)
	, _doctorEdit(new Ui::FlatEdit)
	, _model(model)
	, _row(row)
{
	_countBox->setValue(1);
	_countBox->setMinimum(1);

	_patientIdEdit->setInputValidator(Ui::InputValitor::LetterAndNumber);
	_patientAgeEdit->setInputValidator(Ui::InputValitor::NumberOnly);
	_patientAgeEdit->setMaxLength(3);

	FormGroup * pkgGroup = new FormGroup(this);
	pkgGroup->addRow("器械类型 (*)", _pkgEdit);
	pkgGroup->addRow("数量 (*)", _countBox);

	FormGroup * patientGroup = new FormGroup(this);
	patientGroup->addRow("医生 (*)", _doctorEdit);
	patientGroup->addRow("患者姓名 (*)", _patientNameEdit);
	patientGroup->addRow("住院号", _patientIdEdit);
	patientGroup->addRow("年龄", _patientAgeEdit);
	patientGroup->addRow("性别", _genderCombo);

	if (-1 != _row)
	{
		initData();

		setWindowTitle("修改外来器械");
		_commitButton = new Ui::PrimaryButton("确认", Ui::BtnSize::Small);
		connect(_commitButton, SIGNAL(clicked()), this, SLOT(alter()));
	}
	else
	{
		setWindowTitle("添加外来器械");
		_commitButton = new Ui::PrimaryButton("添加", Ui::BtnSize::Small);
		connect(_commitButton, SIGNAL(clicked()), this, SLOT(accept()));
	}

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(pkgGroup);
	layout->addWidget(patientGroup);
	layout->addWidget(_commitButton);

	resize(parent ? parent->width() / 3 : 360, sizeHint().height());

	QTimer::singleShot(0, this, &AddExtDialog::loadData);
}

void AddExtDialog::initData() {

	_countBox->setValue(_model->item(_row, 1)->text().toInt());
	_doctorEdit->setText(_model->item(_row, 2)->text());
	_patientNameEdit->setText(_model->item(_row, 3)->text());
	_patientIdEdit->setText(_model->item(_row, 4)->text());
	_patientAgeEdit->setText(_model->item(_row, 5)->text());
	_genderCombo->setCurrentText(_model->item(_row, 6)->text());
}

void AddExtDialog::alter() {
	int pkgId = _pkgEdit->currentId();
	int count = _countBox->value();

	QString pkgName = _pkgEdit->currentName();
	QString doctor = _doctorEdit->text();
	QString patient = _patientNameEdit->text();
	QString patientId = _patientIdEdit->text();
	QString patientAge = _patientAgeEdit->text();
	QString patientGender = _genderCombo->currentText();

	if (0 == pkgId || doctor.isEmpty() || patient.isEmpty()) {
		return;
	}

	_model->item(_row, 0)->setText(pkgName);
	_model->item(_row, 0)->setData(pkgId);
	_model->item(_row, 1)->setText(QString::number(count));
	_model->item(_row, 2)->setText(doctor);
	_model->item(_row, 3)->setText(patient);
	_model->item(_row, 4)->setText(patientId);
	_model->item(_row, 5)->setText(patientAge);
	_model->item(_row, 6)->setText(patientGender);
	_model->item(_row, 6)->setData(_genderCombo->currentData());

	QDialog::accept();
}

void AddExtDialog::accept() {
	
	int pkgId = _pkgEdit->currentId();
	int count = _countBox->value();

	QString pkgName = _pkgEdit->currentName();
	QString doctor = _doctorEdit->text();
	QString patient = _patientNameEdit->text();
	QString patientId = _patientIdEdit->text();
	QString patientAge = _patientAgeEdit->text();
	QString patientGender = _genderCombo->currentText();
	QString patientGenderData = _genderCombo->currentData().toString();

	if (0 == pkgId || doctor.isEmpty() || patient.isEmpty()) {
		return;
	}

	QList<QStandardItem *> rowItems;

	QStandardItem *item_pkgName = new QStandardItem();
	item_pkgName->setTextAlignment(Qt::AlignCenter);
	item_pkgName->setText(pkgName);
	item_pkgName->setData(pkgId);
	rowItems.append(item_pkgName);

	QStandardItem *item_count = new QStandardItem();
	item_count->setTextAlignment(Qt::AlignCenter);
	item_count->setText(QString::number(count));
	rowItems.append(item_count);

	QStandardItem *item_doctor = new QStandardItem();
	item_doctor->setTextAlignment(Qt::AlignCenter);
	item_doctor->setText(doctor);
	rowItems.append(item_doctor);

	QStandardItem *item_patient = new QStandardItem();
	item_patient->setTextAlignment(Qt::AlignCenter);
	item_patient->setText(patient);
	rowItems.append(item_patient);

	QStandardItem *item_patientId = new QStandardItem();
	item_patientId->setTextAlignment(Qt::AlignCenter);
	item_patientId->setText(patientId);
	rowItems.append(item_patientId);

	QStandardItem *item_patientAge = new QStandardItem();
	item_patientAge->setTextAlignment(Qt::AlignCenter);
	item_patientAge->setText(patientAge);
	rowItems.append(item_patientAge);

	QStandardItem *item_patientGender = new QStandardItem();
	item_patientGender->setTextAlignment(Qt::AlignCenter);
	item_patientGender->setText(patientGender);
	item_patientGender->setData(patientGenderData);
	rowItems.append(item_patientGender);

	_model->appendRow(rowItems);

	QDialog::accept();
	
}

void AddExtDialog::loadData() {
	_pkgEdit->loadForCategary("2");

	if (-1 != _row)
		_pkgEdit->setCurrentIdPicked(_model->item(_row, 0)->data().toInt(), _model->item(_row, 0)->text());
}
