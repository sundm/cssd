#include "container.h"
#include "ui/composite/titles.h"

#include <QVBoxLayout>
#include <QResizeEvent>
#include <QPainter>
#include <QStyleOption>

#define GOLDEN_SECTION_RATIO 0.618

namespace Ui {

Inner::Inner(QWidget *container /*= nullptr*/):QWidget(container) {
}

Container * Inner::container() const {
	if (auto father = parent()) {
		return qobject_cast<Container *>(father);
	}
	return nullptr;
}

// This should be called whenever the size hint changes  
void Inner::updateSize()
{
	// since Inner is not managed by layout in the Container, it will not
	// automatically adjust its size to fit its contents when size hint is
	// changed (e.g. a child shows/hides), we have to manually do it
	adjustSize();
	// whenever Inner's sizeHint/sizePolicy changes, we should update container's geometry
	// to make the layout system aware that the container has changed (e.g.
	// minimumSizeHint)
	if (auto c = container())
		c->updateGeometry();
}

void Inner::submit() {
}

Container::Container(
	QWidget *inner,
	int position,
	QWidget *parent/* = nullptr*/) : QWidget(parent) {
	setInner(inner, position);
}

void Container::setInner(QWidget *inner, int position/* = Up*/) {
	if (inner) {
		_inner = inner;
		_inner->setParent(this);

		// the default size of QWidget depends on the user's platform and screen geometry,
		// e.g. (640,480), adjust the inner's size is a good practice 
		_inner->resize(_inner->sizeHint());

		// this is not neccesary if the parent/grandparent calls show(),
		// otherwise we have to manually show the inner, since it's a sub-widget
		// rather than a top-level window.
		//_inner->raise();
		//_inner->show();

		_position = position;
	}
}

void Container::accept() {
	emit accepted();
}

QSize Container::sizeHint() const {
	return QSize(960, 520);
}

QSize Container::minimumSizeHint() const {
	return _inner ? _inner->minimumSizeHint() : QSize();
}

void Container::resizeEvent(QResizeEvent *event) {
	updateInnerGeometry(event->size());
	QWidget::resizeEvent(event);
}

void Container::updateInnerGeometry(QSize newSize) {
	int dy = (newSize.height() - _inner->height()) / 2;
	QPoint dp((newSize.width() - _inner->width()) / 2, 0);
	switch (_position) {
	case Container::Top:
		dp.ry() = 0;
		break;
	case Container::Up:
		dp.ry() = dy - std::min(dy, _inner->height() / 2) / 2;
		break;
	case Ui::Container::Center:
		dp.ry() = dy;
		break;
	case Ui::Container::Down:
		dp.ry() = dy * 2 - std::min(dy, _inner->height() / 2) / 2;
		break;
	case Ui::Container::Bottom:
		dp.ry() = dy * 2;
		break;
	default:
		break;
	}
	_inner->move(dp);
}

FadeContainer::FadeContainer(QWidget *inner, int position, QWidget *parent)
	:Container(inner, position, parent) {
	parent->installEventFilter(this);
	resize(parent->size());
	setAttribute(Qt::WA_TranslucentBackground);
}

void FadeContainer::paintEvent(QPaintEvent *event) {
	QPainter p(this);
	p.fillRect(rect(), QColor(0,0,0,138));
}

bool FadeContainer::eventFilter(QObject *obj, QEvent *event) {
	if (obj != parent())
		return false;

	if (event->type() == QEvent::Resize) {
		QResizeEvent *e = static_cast<QResizeEvent*>(event);
		resize(e->size());
	}
	return false;
}

FadeInner::FadeInner(const QString &title, QWidget *parent)
: QWidget(parent)
, _in(new QWidget){
	setupContent(_in);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(new Composite::Title(title));
	layout->addWidget(_in);
	//setMaximumSize(800, 400);
	resize(800, 400);
}

void FadeInner::setupContent(QWidget * w) {
	w->setMinimumHeight(200);
}

void FadeInner::paintEvent(QPaintEvent *event) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

} // namespace Ui
