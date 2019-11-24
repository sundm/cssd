#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
}

class QComboBox;
class QCheckBox;
class TSL1128Reader;

class ConfigRfidReaderDialog : public QDialog
{
	Q_OBJECT

public:
	ConfigRfidReaderDialog(QWidget *parent = Q_NULLPTR);
	void setReader(const int index);
protected:
	void accept() override;

private:
	Ui::FlatEdit * _nameEdit;
	QComboBox *_comBox;
	QPushButton *_connectButton;
	TSL1128Reader *_reader;

	void onClickConnectBtn();
	void onClickDisconnectBtn();
	void setItemEnabled();
};
