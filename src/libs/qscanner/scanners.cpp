#include "scanners.h"
#include "newlandoy20.h"
#include <QCoreApplication>
#include <QTimer>
#include <QKeyEvent>
#include <QDebug>
#include <QWidget>

UsbHidPosScanner::UsbHidPosScanner(BarcodeScannerType type, QObject *parent)
	: IBarcodeScanner(parent)
{
	switch (type) {
	case BarcodeScannerType::USB_HID_POS_NLOY20:
		_gun = new NewLandOY20;
		break;
	default:
		_gun = nullptr;
		break;
	}

	if (_gun) {
		connect(_gun, SIGNAL(dataRecieved(const QString &)),
			this, SLOT(handleBardode(const QString &)), Qt::QueuedConnection);
	}
}

UsbHidPosScanner::~UsbHidPosScanner()
{
}

void UsbHidPosScanner::start()
{
	if (_gun && !_gun->isRunning()) {
		_gun->start(); // start the thread
	}
}

void UsbHidPosScanner::stop()
{
	if (_gun) {
		_gun->stop(); // terminate the thread
	}
}

/** class UsbHidKbwScanner */
UsbHidKbwScanner::UsbHidKbwScanner(QObject *parent /*= Q_NULLPTR*/)
	: IBarcodeScanner(parent)
	, _receiving(false)
	, _timer(new QTimer)
{
	_timer->setInterval(130);
	_timer->setSingleShot(true);
	connect(_timer, SIGNAL(timeout()), this, SLOT(resetKeyPressStatus()));
}

UsbHidKbwScanner::~UsbHidKbwScanner()
{
}

void UsbHidKbwScanner::start()
{
	QCoreApplication::instance()->installEventFilter(this);
}

void UsbHidKbwScanner::stop()
{
	QCoreApplication::instance()->removeEventFilter(this);
}

bool UsbHidKbwScanner::eventFilter(QObject *obj, QEvent *event)
{
	//if (event->type() == QEvent::ShortcutOverride) {
	//	QShortcutEvent *scEvent = static_cast<QShortcutEvent *>(event);
	//	//event->accept();
	//	qDebug("shortcut key press");
	//	return true;
	//	qDebug("shortcut key press %x", scEvent->key());
	//	QKeySequence s = scEvent->key();
	//}
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		//qDebug("Ate key press %x", keyEvent->key());
		QWidget* w = qobject_cast<QWidget *>(obj);
		if (!w) {
			return false;
		}
		//qDebug() << w;

		if (keyEvent->isAutoRepeat()) return false;

		if (_timer->isActive()) { // receiving
			if (Qt::Key_Return == keyEvent->key()) { // FIXME: should consider Key_Enter?
				// end of barcode scan
				if (_curText.length() < 6) {
					return false;
				}
				qDebug() << "barcode:" << _curText;
				//emit dataReceived(_curText);
				handleBardode(_curText);
				_curText.clear();
				return true;
			}
			else {
				//qDebug("Ate key press %d", keyEvent->key());
				_curText += keyEvent->text();
				//qDebug() << _curText;
			}
		}
		else {
			//if (Qt::Key_BracketRight != keyEvent->key()) {
			//if (Qt::Key_F6 != keyEvent->key()) {
			// propagate the event if key is not ']'
			if (!QChar::isDigit(keyEvent->key())) {
				// propagate the event for non-digit keys
				return false;
			}
			_curText = keyEvent->text();
		}

		_timer->start(); // start or restart the timer
		return false;
	}
	else {
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}

void UsbHidKbwScanner::resetKeyPressStatus()
{
	//qDebug() << _curText << " should propagate the original object since timeout";
	qDebug() << "time out";
	_curText.clear();
}
