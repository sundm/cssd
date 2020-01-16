#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
}

class QComboBox;
class TLineEditIP;
class QPushButton;

class DesktopReaderConfigDialog : public QDialog
{
	Q_OBJECT

public:
	DesktopReaderConfigDialog(const QString &address, const int antenna, QWidget *parent = Q_NULLPTR);

protected:
	void accept() override;

private:
	TLineEditIP *_addressEdit;
	QComboBox *_combo;
	QPushButton *_connectButton;

	bool _isConnected;

	void onConnectClicked();
	void onDisConnectClicked();
	void setItemEnabled();
};
