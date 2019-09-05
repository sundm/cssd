#pragma once

#include <QDialog>
#include "core/user.h"
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class QCheckBox;
class GenderComboBox;
class DeptEdit;
class WaitingSpinner;
class AddUserDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddUserDialog(QWidget *parent = Q_NULLPTR);
	void setUserInfo(const Core::User &user);
protected:
	void accept() override;

private:
	Ui::FlatEdit *_nameEdit;
	Ui::FlatEdit *_phoneEdit;
	DeptEdit *_deptEdit;
	GenderComboBox *_genderCombo;
	//QCheckBox *_checkBox;
	WaitingSpinner *_waiter;

	int _userId;
	bool _isModify;
};
