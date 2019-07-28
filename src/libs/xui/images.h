#pragma once

#include "xui_global.h"
#include <QWidget>

class QPixmap;

enum class BgFit {
	Center,
	Fill,
	Fit,
	Stretch,
	Tile     // no scale, tipically for tiny pics
};

class XUI_EXPORT XPicture : public QWidget
{
	Q_OBJECT
public:
	using BgColor = QColor;
	XPicture(const QString & fileName, QWidget *parent = nullptr);
	XPicture(QWidget *parent = nullptr);
	~XPicture() = default;

	void setBgFit(BgFit fit);
	void setBgColor(BgColor color);
	void setFixedHeight(int h = 0);
	void setImage(const QString &fileName);
	QString fileName() const;

signals:
	void clicked();

protected:
	void paintEvent(QPaintEvent *event) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	QSize sizeHint() const override;

private:
	void drawCenter(QPainter *);
	void drawFill(QPainter *);
	void drawFit(QPainter *);
	void drawStretch(QPainter *);
	void drawTile(QPainter *);
	void drawBg(QPainter *);
	
	std::unique_ptr<QPixmap> _pixmap;
	QString _fileName;
	BgFit _fit = BgFit::Fit;
	BgColor _color;
};
