#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class QComboBox;
class WaitingSpinner;
class AddProgramDialog : public QDialog, public JsonHttpClient
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
	WaitingSpinner *_waiter;
};
