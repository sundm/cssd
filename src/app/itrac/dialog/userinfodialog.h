#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
}

class UserInfoDialog : public QDialog
{
	Q_OBJECT

public:
	UserInfoDialog(QWidget *parent = Q_NULLPTR);

private:
	void changePwd();

	Ui::FlatEdit *_oldPwdEdit;
	Ui::FlatEdit *_newPwdEdit;
	Ui::FlatEdit *_newPwdConfirmEdit;
};
