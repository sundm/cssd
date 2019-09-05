#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class QCheckBox;
class WaitingSpinner;
class AddInstrumentDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddInstrumentDialog(QWidget *parent = Q_NULLPTR);
	void setInfo(const QString &id, const QString &name, const QString &pinyin, const bool isVIP);
protected:
	void accept() override;

private:
	Ui::FlatEdit *_nameEdit;
	Ui::FlatEdit *_pinyinEdit;
	QCheckBox *_checkBox;
	WaitingSpinner *_waiter;

	bool _isModify;
	QString _instrumentId;
};
