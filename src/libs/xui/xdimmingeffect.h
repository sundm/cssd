#pragma once

#include "xui_global.h"
#include <QGraphicsEffect>

class XUI_EXPORT XDimmingEffect : public QGraphicsEffect
{
	Q_OBJECT

public:
	XDimmingEffect(QObject *parent = nullptr);
	~XDimmingEffect();

protected:
	void draw(QPainter* painter) override;
};
