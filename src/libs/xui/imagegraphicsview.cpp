#include "imagegraphicsview.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMessageBox>

ImageGraphicsView::ImageGraphicsView(QWidget *parent)
	: QGraphicsView(parent)
	, _scene(new QGraphicsScene(this))
	, _totalScaleFactor(1)
	, _factor(0)
{
	viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
	setDragMode(ScrollHandDrag);
	setFrameStyle(QFrame::NoFrame);

	_scene->setBackgroundBrush(QColor::fromRgb(245, 246, 247));
	setScene(_scene);
}

ImageGraphicsView::~ImageGraphicsView() { }


int ImageGraphicsView::getFactor() {
	return _factor;
}


void ImageGraphicsView::setFactor(int fact) {
	if (fact == 0)
		_factor = 0;
	else
		_factor += fact;
}


void ImageGraphicsView::load(const QString &fileName)
{
	if (!fileName.isEmpty()) {
		QFile f(fileName);
		if (!f.open(QIODevice::ReadOnly)) {
			QMessageBox::critical(this, "图片预览", "无法加载指定的图片对象");
			return;
		}

		QPixmap pixmap(fileName);
		QGraphicsPixmapItem *item = _scene->addPixmap(pixmap);
		item->setAcceptTouchEvents(true);
	}
}

bool ImageGraphicsView::viewportEvent(QEvent *event)
{
	switch (event->type()) {
	case QEvent::TouchBegin:
	case QEvent::TouchUpdate:
	case QEvent::TouchEnd:
	{
		QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
		QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
		if (touchPoints.count() == 2) {
			// determine scale factor
			const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
			const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
			qreal currentScaleFactor =
				QLineF(touchPoint0.pos(), touchPoint1.pos()).length()
				/ QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();
			if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
				// if one of the fingers is released, remember the current scale
				// factor so that adding another finger later will continue zooming
				// by adding new scale factor to the existing remembered value.
				_totalScaleFactor *= currentScaleFactor;
				currentScaleFactor = 1;
			}
			setTransform(QTransform().scale(_totalScaleFactor * currentScaleFactor,
				_totalScaleFactor * currentScaleFactor));
		}
		return true;
	}
	default:
		break;
	}
	return QGraphicsView::viewportEvent(event);
}

void ImageGraphicsView::wheelEvent(QWheelEvent *e)
{
	int distancia = e->delta() / 15 / 8;
	qreal val;

	if (distancia != 0) {
		val = pow(1.2, distancia);
		this->scale(val, val);
		if (distancia > 0)
			_factor++;
		else
			_factor--;
	}
}
