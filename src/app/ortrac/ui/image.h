#pragma once

#include <QWidget>

class QPixmap;

namespace Ui {

enum class BgFit {
	Center,
	Fill,
	Fit,
	Stretch,
	Tile     // no scale, tipically for tiny pics
};

class Picture : public QWidget
{
	Q_OBJECT
public:
	using BgColor = QColor;
	Picture(const QString & fileName, QWidget *parent = nullptr);
	~Picture() = default;

	void setBgFit(BgFit fit);
	void setBgColor(BgColor color);
	void setFixedHeight(int h = 0);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	void drawCenter(QPainter *);
	void drawFill(QPainter *);
	void drawFit(QPainter *);
	void drawStretch(QPainter *);
	void drawTile(QPainter *);
	
	std::unique_ptr<QPixmap> _pixmap;
	BgFit _fit = BgFit::Fit;
	BgColor _color = Qt::white;
};

} // namespace Ui 
