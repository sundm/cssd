#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"
#include "core/user.h"

namespace Ui {
	class FlatEdit;
}

class WaitingSpinner;

class ResetUserPWDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	ResetUserPWDialog(const Core::User &user, QWidget *parent = Q_NULLPTR);

private:
	void resetPwd();

	int _userId;
	WaitingSpinner *_waiter;
};
