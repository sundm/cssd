#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class UserInfoDialog : public QDialog, public JsonHttpClient
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
