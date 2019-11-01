#pragma once

#include <QDialog>

class QComboBox;
class QCheckBox;
class ConfigRfidReaderDialog : public QDialog
{
	Q_OBJECT

public:
	ConfigRfidReaderDialog(QWidget *parent = Q_NULLPTR);

protected:
	void accept() override;

private:
	QComboBox *_comBox;
	QCheckBox *_rememberMeBox;

	void onClickConnectBtn();
	void onClickDisconnectBtn();
};
