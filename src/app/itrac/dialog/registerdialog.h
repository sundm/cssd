#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
}

class RegisterDialog : public QDialog
{
	Q_OBJECT

public:
	RegisterDialog(const QString &src, QWidget *parent = Q_NULLPTR);

private:
	Ui::FlatEdit *_srcCodeEdit;
	Ui::FlatEdit *_codeEdit;

	void onRegisterBtnClicked();
	void writeKeyToFile();
};
