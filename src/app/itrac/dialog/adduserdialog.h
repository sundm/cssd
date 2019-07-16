#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class QCheckBox;
class GenderComboBox;
class DeptEdit;

class AddUserDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddUserDialog(QWidget *parent = Q_NULLPTR);

protected:
	void accept() override;

private:
	Ui::FlatEdit *_nameEdit;
	Ui::FlatEdit *_phoneEdit;
	DeptEdit *_deptEdit;
	GenderComboBox *_genderCombo;
	QCheckBox *_checkBox;
};
