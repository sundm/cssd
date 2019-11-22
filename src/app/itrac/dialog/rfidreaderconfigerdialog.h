#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
}

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
	Ui::FlatEdit * _nameEdit;
	QComboBox *_comBox;

	void onClickConnectBtn();
	void onClickDisconnectBtn();
};
