#include "cssd_overlay.h"

#include "ui/composite/titles.h"

#include <QVBoxLayout>
#include <QPropertyAnimation>

CssdOverlay::CssdOverlay(const QString &title, QWidget *parent /*= nullptr*/)
	: BasicOverlay(parent)
	, _layout(new QVBoxLayout(this))
	//, _anim(new QPropertyAnimation(this, "geometry"))
	, _anim(new QPropertyAnimation(this, "pos"))
	, _panel(nullptr) {

	_layout->setContentsMargins(0, 0, 0, 0);
	Composite::Title *t = new Composite::Title(title);
	_layout->addWidget(t);

	_anim->setDuration(150);
	QRect parent_rc = parentWidget()->rect();
	//_anim->setStartValue(QRect(0, 0, 0, parent_rc.height()));
	//_anim->setEndValue(parent_rc);
	_anim->setStartValue(QPoint(-parent_rc.width(), 0));
	_anim->setEndValue(QPoint());

	connect(_anim, &QPropertyAnimation::finished, this, [this] {
		if (_anim->direction() == QPropertyAnimation::Backward) {
			close();
		}
	});

	connect(t, SIGNAL(canceled()), this, SLOT(cancel()));
}

void CssdOverlay::setCentralPanel(CssdOverlayPanel *panel) {
	panel->setParent(this); // FIXME: is this neccesary?
	_layout->insertWidget(1, panel, 0/*, Qt::AlignHCenter*/);
	_panel = panel;
}

void CssdOverlay::showAnimated() {
	_anim->start();
	show();
}

void CssdOverlay::cancel() {
	if (_panel->accept())
		closeAnimated();
}

void CssdOverlay::setChildrenVisible(bool b) {
	for (int i = 0; i < _layout->count(); ++i)
	{
		QWidget *widget = _layout->itemAt(i)->widget();
		if (widget) widget->setVisible(b);
	}
	show();
}

void CssdOverlay::closeAnimated() {
	_anim->setDirection(QPropertyAnimation::Backward);
	_anim->start();
}

