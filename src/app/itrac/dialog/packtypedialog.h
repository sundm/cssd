#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class QSpinBox;
class WaitingSpinner;

class AddPackTypeDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddPackTypeDialog(QWidget *parent = Q_NULLPTR);
	void setInfo(const QString &name, const int &id, const int &valid, const int &max);
protected:
	void accept() override;

private:
	Ui::FlatEdit *_nameEdit;
	QSpinBox *_validBox;
	QSpinBox *_maxBox;
	
	WaitingSpinner *_waiter;
	int _id;
	bool _isModify;
};
