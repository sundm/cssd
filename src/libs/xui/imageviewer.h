#pragma once

#include "xui_global.h"
#include <QWidget>

class ImageGraphicsView;

class XUI_EXPORT ImageViewer : public QWidget
{
	Q_OBJECT

public:
	ImageViewer(const QString &fileName = QString(), QWidget *parent = Q_NULLPTR);
	~ImageViewer();

private slots:
	void zoomIn();
	void zoomOut();
	void zoomReset();
	void fitWindow();

private:
	ImageGraphicsView * _view;
};
