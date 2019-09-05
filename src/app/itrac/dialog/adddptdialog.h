#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class QCheckBox;
class WaitingSpinner;
class AddDptDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddDptDialog(QWidget *parent = Q_NULLPTR);
	void setDtpInfo(const QString& id, const QString& name, const QString& pinyin, const QString& phone);
protected:
	void accept() override;

private:
	Ui::FlatEdit *_nameEdit;
	Ui::FlatEdit *_pinyinEdit;
	Ui::FlatEdit *_phoneEdit;
	WaitingSpinner *_waiter;

	bool _isModify;
	int _dptId;
};
