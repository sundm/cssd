#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
	class PrimaryButton;
}

class QSpinBox;
class PackageEdit;
class GenderComboBox;
class QStandardItemModel;

class AddExtDialog : public QDialog
{
	Q_OBJECT

public:
	AddExtDialog(QWidget *parent, QStandardItemModel *model, int row = -1);

protected:
	void accept() override;

private slots:
	void alter();
	
private:
	void initData();
	void loadData();

	PackageEdit * _pkgEdit;
	QSpinBox * _countBox;
	GenderComboBox *_genderCombo;
	Ui::FlatEdit * _patientNameEdit;
	Ui::FlatEdit * _patientAgeEdit;
	Ui::FlatEdit * _patientIdEdit;
	Ui::FlatEdit * _doctorEdit;
	Ui::PrimaryButton *_commitButton;

	QStandardItemModel *_model;

	int _row;
};
