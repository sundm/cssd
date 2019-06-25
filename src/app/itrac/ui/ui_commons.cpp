#include "ui_commons.h"

#include <qnamespace.h>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

namespace Ui {

	void addSpacer(QLayout *layout, Qt::Orientation orient, int space/* = 40*/)
	{
		bool vertical = orient == Qt::Vertical;
		layout->addItem(new QSpacerItem(
			vertical ? 0 : space,
			vertical ? space : 0,
			QSizePolicy::Minimum,
			QSizePolicy::Fixed));
	}

	void addPrimaryShortcut(QWidget *w, int key, const char *method) {
		QAction* action = new QAction(w);
		action->setShortcut(QKeySequence(key));
		action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		w->connect(action, SIGNAL(triggered()), method);
		w->addAction(action);
	}

	QWidget *createSeperator(Qt::Orientation orient)
	{
		QFrame *line = new QFrame;
		line->setFrameShape(Qt::Vertical == orient ? QFrame::VLine : QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		return line;
	}

} // namespace Ui

Stylable::Stylable(QWidget *parent /*= nullptr*/)
	:QWidget(parent) {
}

void Stylable::paintEvent(QPaintEvent *e) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

BasicOverlay::BasicOverlay(QWidget *parent /*= nullptr*/)
	: Stylable(parent) {
	setAttribute(Qt::WA_DeleteOnClose);
	parent->installEventFilter(this);
	resize(parent->size());
}

bool BasicOverlay::eventFilter(QObject *obj, QEvent *event) {
	if (obj != parent())
		return false;

	if (event->type() == QEvent::Resize) {
		QResizeEvent *e = static_cast<QResizeEvent*>(event);
		resize(e->size());
	}
	return false;
}
