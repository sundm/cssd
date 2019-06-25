#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
}

class QSpinBox;
class FormGroup;
class PackageEdit;
class VendorComboBox;
class GenderComboBox;

class ExtRecvDialog : public QDialog
{
	Q_OBJECT

public:
	ExtRecvDialog(QWidget *parent);

protected:
	void accept() override;
	
private:
	void loadData();

	PackageEdit * _pkgEdit;
	QSpinBox * _countBox;
	VendorComboBox * _vendorCombo;
	GenderComboBox *_genderCombo;
	Ui::FlatEdit * _senderEdit;
	Ui::FlatEdit * _senderPhoneEdit;
	Ui::FlatEdit * _patientNameEdit;
	Ui::FlatEdit * _patientAgeEdit;
	Ui::FlatEdit * _patientIdEdit;
	Ui::FlatEdit * _doctorEdit;
};
