#include "image.h"
#include <QPixmap>
#include <QPainter>

namespace Ui {

	Picture::Picture(const QString & fileName, QWidget *parent/* = nullptr*/) : QWidget(parent),
		_pixmap(std::make_unique<QPixmap>(fileName)) {
		// drawPixmap() is faster on-screen while drawImage()
		// may be faster on a QPrinter or other devices.
	}

	void Picture::setBgFit(BgFit fit) {
		_fit = fit;
	}

	void Picture::setBgColor(BgColor color) {
		_color = color;
	}

	void Picture::setFixedHeight(int h/* = 0*/) {
		QWidget::setFixedHeight(h ? h : _pixmap->height());
	}

	void Picture::paintEvent(QPaintEvent *event) {
		QPainter p(this);
		p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
		switch (_fit) {
		case BgFit::Center: drawCenter(&p); break;
		case BgFit::Fill: drawFill(&p); break;
		case BgFit::Fit: drawFit(&p); break;
		case BgFit::Stretch: drawStretch(&p); break;
		case BgFit::Tile: drawTile(&p); break;
		}
	}

	// no scale, with margins around smaller pics, only the center part of larger pics is visible
	void Picture::drawCenter(QPainter *p) {
		int dx = (width() - _pixmap->width()) / 2;
		int dy = (height() - _pixmap->height()) / 2;
		int tx = dx < 0 ? 0 : dx;
		int ty = dy < 0 ? 0 : dy;
		QRect imgRect = _pixmap->rect();
		QPoint imgCenter = imgRect.center();
		(imgRect &= rect()).moveCenter(imgCenter);

		p->drawPixmap(QPoint(tx, ty), *_pixmap, imgRect);
	}

	// uniform scale, pin the topleft of the pic, scale unitil
	// width or height fits the screen and no margins. Qt::KeepAspectRatioByExpanding
	void Picture::drawFill(QPainter *p) {
		qreal hratio = static_cast<qreal>(width()) / _pixmap->width();
		qreal vratio = static_cast<qreal>(height()) / _pixmap->height();
		bool fitH = hratio > vratio;
		QRectF src = QRectF(0., 0.,
			fitH ? _pixmap->width() : width() / vratio,
			fitH ? height() / hratio : _pixmap->height());
		p->drawPixmap(rect(), *_pixmap, src);
	}

	// uniform scale, pin the center, scale until the
	// width or height fits the screen and all of the pic is visible. Qt::KeepAspectRatio
	void Picture::drawFit(QPainter *p) {
		// use floating point precision to improve the quality
		qreal hratio = static_cast<qreal>(width()) / _pixmap->width();
		qreal vratio = static_cast<qreal>(height()) / _pixmap->height();
		bool fitV = hratio > vratio;
		QRectF trc = QRectF(0., 0.,
			fitV ? _pixmap->width()*vratio : width(),
			fitV ? height() : _pixmap->height()*hratio);
		trc.moveCenter(rect().center());
		p->drawPixmap(trc.toRect(), *_pixmap);
	}

	// non-uniform scale, no margins, all of the pic is visible. Qt::IgnoreAspectRatio
	void Picture::drawStretch(QPainter *p) {
		p->drawPixmap(rect(), *_pixmap);
	}

	void Picture::drawTile(QPainter *p) {
		// maybe use drawTiledPixmap is better
		//p->drawTiledPixmap(rect(),*_pixmap);
		int x = 0;
		int iw = _pixmap->width(), ih = _pixmap->height();
		do {
			p->drawPixmap(QPoint(x, 0), *_pixmap);

			int y = ih;
			while (y < height()) {
				p->drawPixmap(QPoint(x, y), *_pixmap);
				y += ih;
			}

			x += iw;
		} while (x < width());
	}

} // namespace Ui 
