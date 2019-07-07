#include "qscanner.h"
#include "scanners.h"

/** class Scanable */

Scanable::Scanable() {
	bind();
}

void Scanable::bind() {
	BarcodeScannerFactory::setScanable(this);
}

void Scanable::setCandidate(Scanable *c) {
	_candidate = c;
}

Scanable::~Scanable() {
	Scanable *s = BarcodeScannerFactory::scannable();
	if (s == this) {
		BarcodeScannerFactory::setScanable(_candidate ? _candidate : nullptr);
	}
	//BarcodeScannerFactory::setScanable(_candidate ? _candidate : nullptr);
}


IBarcodeScanner::IBarcodeScanner(QObject *parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, _s(nullptr)
{
}

/** class IBarcodeScanner */

void IBarcodeScanner::handleBardode(const QString &code)
{
	if (_s) {
		_s->handleBarcode(code);
	}
}


/** class QScannerFactory */

IBarcodeScanner * BarcodeScannerFactory::activeScanner = nullptr;

void BarcodeScannerFactory::setScanable(Scanable *s) {
	if (activeScanner && activeScanner->_s != s) {
		activeScanner->_s = s;
	}
}

Scanable * BarcodeScannerFactory::scannable() {
	return activeScanner->_s;
}

IBarcodeScanner* BarcodeScannerFactory::Create(BarcodeScannerType type, QObject *parent)
{
	if (activeScanner) {
		delete activeScanner;
		activeScanner = nullptr;
	}

	if (BarcodeScannerType::HID_KBW == type) {
		activeScanner = new UsbHidKbwScanner(parent);
	}
	else {
		activeScanner = new UsbHidPosScanner(type, parent);
	}
	
	return activeScanner;
}

//bool QScannerFactory::connect(const QObject * receiver, const char * method, Qt::ConnectionType type /*= Qt::AutoConnection*/)
//{
//	if (!activeScanner) {
//		return false;
//	}
//	QObject::disconnect(activeScanner, SIGNAL(dataRecieved(const QString&)), 0, 0);
//
//	return QObject::connect(activeScanner, SIGNAL(dataRecieved(const QString&)),
//		receiver, method, type);
//}
//
//bool QScannerFactory::connect(QObject *context, std::function<void(const QString &)> func)
//{
//	if (!activeScanner) {
//		return false;
//	}
//	QObject::disconnect(activeScanner, SIGNAL(dataRecieved(const QString&)), 0, 0);
//
//	return QObject::connect(activeScanner, &IBarcodeScanner::dataRecieved, context, func);
//}