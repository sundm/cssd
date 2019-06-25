#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
}

class QComboBox;

class AddProgramDialog : public QDialog
{
	Q_OBJECT

public:
	AddProgramDialog(QWidget *parent = Q_NULLPTR);

protected:
	void accept() override;

private:
	QComboBox *_typeCombo;
	Ui::FlatEdit *_nameEdit;
	Ui::FlatEdit *_descEdit;
};
