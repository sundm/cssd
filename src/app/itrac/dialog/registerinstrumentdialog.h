#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class QCheckBox;
class QTextEdit;
class QSpinBox;

class RegisterInstrumentDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	RegisterInstrumentDialog(QWidget *parent = Q_NULLPTR);
	void setInfo(const QString&card_id, const QString&pkg_id, const QString&ins_name, const QString&ins_id, int state);
protected:
	void accept() override;

signals:
	void sendData(int, int);

private:
	Ui::FlatEdit *_nameEdit;
	QCheckBox *_checkLostBox;
	QCheckBox *_checkBrokenBox;
	
	QString _pkg_id;
	QString _card_id;
	QString _ins_id;
	QString _op_id;
	int _state;
	int _state_f;
	int _state_e;
};