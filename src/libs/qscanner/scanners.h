#pragma once

#include "qscanner.h"

class IBarcodeGun;
class UsbHidPosScanner : public IBarcodeScanner
{
	Q_OBJECT

public:
	UsbHidPosScanner(BarcodeScannerType type, QObject *parent = Q_NULLPTR);
	~UsbHidPosScanner();

	void start();
	void stop();

private:
	IBarcodeGun * _gun;
};

class QTimer;
class UsbHidKbwScanner : public IBarcodeScanner
{
	Q_OBJECT

public:
	UsbHidKbwScanner(QObject *parent = Q_NULLPTR);
	~UsbHidKbwScanner();

	void start();
	void stop();

protected:
	bool eventFilter(QObject *obj, QEvent *event);

private slots:
	void resetKeyPressStatus();

private:
	QTimer * _timer;
	QString _curText;
	bool _receiving;
};
