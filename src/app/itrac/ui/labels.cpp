#include "labels.h"

#include <QVariant>
#include <QPropertyAnimation>
#include <QPainter>

namespace Ui {

Title::Title(const QString text, QWidget *parent/* = nullptr*/)
	: QLabel(text, parent) {
	setAlignment(Qt::AlignCenter);
	setProperty("title", true);
}

Description::Description(const QString text, QWidget *parent /*= nullptr*/)
	: QLabel(text, parent) {
	setAlignment(Qt::AlignCenter);
	setProperty("desc", true);
}

NormalLabel::NormalLabel(const QString text, QWidget *parent /*= nullptr*/)
	: QLabel(text, parent) {
	setAlignment(Qt::AlignCenter);
}

ErrorLabel::ErrorLabel(const QString text, QWidget *parent /*= nullptr*/) 
	: QLabel(text, parent)
	, _anim(std::make_unique<QPropertyAnimation>(this, "pos")){
	// Using a QPalette isn't guaranteed to work for all styles,
	// because style authors are restricted by the different platforms'
	// guidelines and by the native theme engine, thus we use stylesheet here
	setStyleSheet("QLabel{color:rgb(170,0,0);}");

	_anim->setDuration(300);
}

void ErrorLabel::shake(const QString &error)
{
	if (QAbstractAnimation::Running == _anim->state())
		return;

	setText(error);
	show();

	int x = pos().x(), y= pos().y();
	_anim->setStartValue(pos());
	_anim->setKeyValueAt(0.3, QPoint(x + 10, y));
	_anim->setKeyValueAt(0.5, pos());
	_anim->setKeyValueAt(0.8, QPoint(x + 5, y));
	_anim->setEndValue(pos());
	_anim->start();
}

CheckableImageLabel::CheckableImageLabel(const QPixmap &pixmap, QWidget *parent /*= nullptr*/)
	:_checked(false), _pixmap(":/res/checked.png") {
	setPixmap(pixmap);
}

void CheckableImageLabel::setChecked(bool b) {
	if (_checked != b) {
		_checked = b;
		update();
	}
}

void CheckableImageLabel::paintEvent(QPaintEvent *e) {
	QLabel::paintEvent(e);
	if (_checked) {
		QPainter p(this);
		QRect rc = _pixmap.rect();
		rc.moveCenter(rect().center());
		p.drawPixmap(rc, _pixmap);
	}
}

} // namespace Ui
