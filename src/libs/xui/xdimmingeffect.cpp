#include <QPainter>
#include "xdimmingeffect.h"

XDimmingEffect::XDimmingEffect(QObject *parent)
	: QGraphicsEffect(parent)
{
}

XDimmingEffect::~XDimmingEffect()
{
}

void XDimmingEffect::draw(QPainter* painter)
{
	QPixmap pixmap;
	QPoint offset;
	if (sourceIsPixmap()) // No point in drawing in device coordinates (pixmap will be scaled anyways)
		pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);
	else // Draw pixmap in device coordinates to avoid pixmap scaling;
	{
		pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset);
		painter->setWorldTransform(QTransform());
	}
	painter->setBrush(QColor(128, 128, 128, 255)); // black bg
	painter->drawRect(pixmap.rect());
	painter->setOpacity(0.5);
	painter->drawPixmap(offset, pixmap);
}
