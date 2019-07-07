#include "loader.h"

#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace Ui {


Source::Source(QWidget *parent /*= nullptr*/)
	:QWidget(parent) {
}

void Source::hideAnimated() {
	hide();
	emit hidden();
}

void Source::showAnimated() {
	show();
	emit shown();
}

Loader::Loader(QWidget *parent)
	: QWidget(parent) {
}

void Loader::setSource(Source *src) {
	src->setParent(this);

	if (_source) {
		_source->hide();
		// no resize event triggerred, manually resize the new source
		src->resize(size());
		// this is necessary when there is an old source, but not needed for
		// the initial source, because the Loader::show/showMaximized will be called
		src->show(); 
	}

	_source.reset(src); // the old managed object (if exists) is deleted

	// since no layout is put on Loader, the minimum size hint is an invalid size
	setMinimumSize(src->minimumSizeHint());
}

void Loader::setSourceAnimated(Source *src, AnimMode animMode, int pause) {
	if (nullptr == _source)
		return setSource(src);

	src->setParent(this);
	src->resize(size());

	if (animMode == AnimMode::Custom) { // use source animations
		connect(_source.get(), &Source::hidden, this, [=] {
			_source.reset(src);
			QTimer::singleShot(pause, [src] {src->showAnimated(); });
		});
		_source->hideAnimated();
	}
	else {
		animate(src, animMode);
	}
}

void Loader::resizeEvent(QResizeEvent *event) {

	if (_source) {
		_source->resize(size());
	}
	QWidget::resizeEvent(event);
}

void Loader::animate(Source *src, AnimMode animMode) {
	const int duration = 300;
	QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect;
	QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity");
	anim->setDuration(duration);

	if (AnimMode::FadeIn == animMode) {
		_source->hide();
		src->setGraphicsEffect(eff);
		src->show();
		_source.reset(src);
		anim->setStartValue(qreal(0));
		anim->setEndValue(qreal(1));
		connect(anim, &QPropertyAnimation::finished, this, [=] {
			src->setGraphicsEffect(nullptr);
		});
		anim->start(QPropertyAnimation::DeleteWhenStopped);
	}
	else if (AnimMode::FadeOutIn == animMode) {
		_source->setGraphicsEffect(eff); // _source takes ownership of effect.

		anim->setStartValue(qreal(1));
		anim->setEndValue(qreal(0));
		connect(anim, &QPropertyAnimation::finished, this, [=] {
			if (anim->direction() == QAbstractAnimation::Forward) {
				// transfer the ownership before reset(), or it'll be deleted
				src->setGraphicsEffect(eff); // now the new source takes ownership of effect
				src->show();
				_source.reset(src);
				anim->setDirection(QAbstractAnimation::Backward);
				anim->start(QPropertyAnimation::DeleteWhenStopped);
			}
			else {
				src->setGraphicsEffect(nullptr); // delete the effect on the new source
			}
		});
		anim->start();
	}
}

//QSize Loader::minimumSizeHint() const {
//	return _source ? _source->minimumSizeHint() : QSize();
//}

} // namespace Ui
