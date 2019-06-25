#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
}

class QCheckBox;

class AddInstrumentDialog : public QDialog
{
	Q_OBJECT

public:
	AddInstrumentDialog(QWidget *parent = Q_NULLPTR);

protected:
	void accept() override;

private:
	Ui::FlatEdit *_nameEdit;
	Ui::FlatEdit *_pinyinEdit;
	QCheckBox *_checkBox;
};
