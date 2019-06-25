#include "scaneater.h"
#include <qtimer>
#include <qkeyevent>
#include <qdebug>

ScanEater::ScanEater(QObject *parent)
	:QObject(parent), _receiving(false)
{
	_timer = new QTimer();
	_timer->setInterval(30);
	_timer->setSingleShot(true);
	connect(_timer, SIGNAL(timeout()), this, SLOT(resetKeyPressStatus()));
}

void ScanEater::resetKeyPressStatus()
{
	qDebug() << _curText << " should propagate the original object since timeout";
	_curText.clear();
}

bool ScanEater::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		//qDebug("Ate key press %x", keyEvent->key());

		if (keyEvent->isAutoRepeat()) return false;

		if (_timer->isActive()) { // receiving
			if (Qt::Key_Return == keyEvent->key()) {
				// end of barcode scan
				qDebug() << _curText;
				emit dataReceived(_curText);
				_curText.clear();
			}
			else {
				//qDebug("Ate key press %d", keyEvent->key());
				_curText += keyEvent->text();
			}
		}
		else {
			//if (Qt::Key_BracketRight != keyEvent->key()) {
				// propagate the event if key is not ']'
			if (!QChar::isDigit(keyEvent->key())) {
				// propagate the event for non-digit keys
				return false;
			}
			_curText = keyEvent->text();
		}

		_timer->start(); // start or restart the timer
		return true;
	}
	else {
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}