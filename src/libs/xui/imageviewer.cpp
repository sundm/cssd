#include "imageviewer.h"
#include "imagegraphicsview.h"

#include <QFrame>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>

namespace {

QToolButton *createToolButton(const QString &fileName, QWidget *parent = nullptr) {
	QToolButton *btn = new QToolButton(parent);
	btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
	btn->setIcon(QIcon(fileName));
	btn->setIconSize(QSize(48, 48));
	return btn;
}

}

ImageViewer::ImageViewer(const QString &fileName, QWidget *parent)
	: QWidget(parent)
	, _view(new ImageGraphicsView(this))
{
	setAttribute(Qt::WA_DeleteOnClose);

	QFrame *frame = new QFrame(this);
	QToolButton *zoomInButton = createToolButton(":/xui/zoomin.png", frame);
	QToolButton *zoomResetButton = createToolButton(":/xui/zoom0.png", frame);
	QToolButton *zoomOutButton = createToolButton(":/xui/zoomout.png", frame);
	//QToolButton *fitWindowButton = createToolButton(":/xui/zoomin.png", frame);

	QHBoxLayout *frameLayout = new QHBoxLayout(frame);
	frameLayout->addStretch();
	frameLayout->addWidget(zoomInButton);
	frameLayout->addWidget(zoomResetButton);
	frameLayout->addWidget(zoomOutButton);
	//frameLayout->addWidget(fitWindowButton);
	frameLayout->addStretch();
	frameLayout->setSpacing(20);

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->addWidget(_view);
	vLayout->addWidget(frame);
	vLayout->setContentsMargins(0, 0, 0, 0);

	connect(zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));
	connect(zoomResetButton, SIGNAL(clicked()), this, SLOT(zoomReset()));
	connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));
	//connect(fitWindowButton, SIGNAL(clicked()), this, SLOT(fitWindow()));

	_view->load(fileName);
}

ImageViewer::~ImageViewer()
{
}

void ImageViewer::zoomIn()
{
	_view->scale(1.2, 1.2);
	_view->setFactor(1);
}

void ImageViewer::zoomOut()
{
	_view->scale(1 / 1.2, 1 / 1.2);
	_view->setFactor(-1);
}

void ImageViewer::zoomReset()
{
	qreal val = pow(1.2, _view->getFactor());
	_view->scale(1/val, 1/val);
	_view->setFactor(0);
}

void ImageViewer::fitWindow()
{

}
