#ifndef GRAPHICSVIEW_HPP
#define GRAPHICSVIEW_HPP

#include <QGraphicsView>
#include <QWheelEvent>
#include <QtMath>

class QGraphicsScene;
class ImageGraphicsView : public QGraphicsView {

    private:
		QGraphicsScene * _scene;
        int _factor;
		qreal _totalScaleFactor;

    public:

        ImageGraphicsView(QWidget* parent = 0);
        ~ImageGraphicsView();

        int getFactor();
        void setFactor(int fact);
		void load(const QString &fileName);

    protected:
		bool viewportEvent(QEvent *event);
        void wheelEvent(QWheelEvent* e);
};

#endif // GRAPHICSVIEW_HPP
