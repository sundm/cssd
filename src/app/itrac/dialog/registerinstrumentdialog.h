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

protected:
	void accept() override;

private slots:
	void checklostChanged(int);
	void ckeckBrokenChanged(int);

private:
	Ui::FlatEdit *_nameEdit;
	QCheckBox *_checkLostBox;
	QSpinBox *_lostNumber;

	QCheckBox *_checkBrokenBox;
	QTextEdit *_brokenInfo;

};
