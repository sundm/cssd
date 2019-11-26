#pragma once

#include "core/ds.h"
#include "ui/loader.h"
#include <QGraphicsObject>
#include <QGraphicsView>

class DoorPlate : public QGraphicsObject {
	Q_OBJECT
public:
	DoorPlate();

protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
	void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
	QRectF boundingRect() const override;

signals:
	void clicked();

private slots:
	void updateInternalData();

private:
	struct Data {
		QString text;
		QColor color;
		Data(const QString &t, const QColor& c) {
			text = t;
			color = c;
		}
	};

	void createInterlData();
	void createTransformStates();

	QPolygon _polygon;
	Circulator<Data> _data;
	bool _clickable;

	static constexpr int w = 230;
	static constexpr int h = 60;
	static constexpr int sw = 40;
	static constexpr int adjust = 3;
	static constexpr QPoint points[5] = { // QPoint is an aggregate type as well as literal type
		QPoint(0, -h/2),
		QPoint(w-sw, -h/2),
		QPoint(w, 0),
		QPoint(w-sw, h/2),
		QPoint(0, h/2)
	};
};

class DoorPlatePanel : public QGraphicsView
{
	Q_OBJECT
public:
	DoorPlatePanel(QWidget *parent = nullptr);
signals:
	void flipped();
};

class QAbstractButton;
class QGridLayout;
class QSignalMapper;
class CssdAreaPanel : public Ui::Source {
	Q_OBJECT
public:
	CssdAreaPanel(QWidget *parent = nullptr);
public slots:
	void clickCallback(int id);
protected:
	QAbstractButton * addButton(int id,
		const QString &file,
		const QString &text,
		const QString &desc);
private:
	QGridLayout * _layout;
	QSignalMapper * _signalMapper;
	int _count;
	static constexpr int columnCount = 3;
};

class PollutedAreaPanel : public CssdAreaPanel {
	Q_OBJECT
public:
	PollutedAreaPanel(QWidget *parent = nullptr);
};

class CleanAreaPanel : public CssdAreaPanel {
	Q_OBJECT
public:
	CleanAreaPanel(QWidget *parent = nullptr);
};

class AsepsisAreaPanel : public CssdAreaPanel {
	Q_OBJECT
public:
	AsepsisAreaPanel(QWidget *parent = nullptr);
};

class OperatingAreaPanel : public CssdAreaPanel {
	Q_OBJECT
public:
	OperatingAreaPanel(QWidget *parent = nullptr);
};

class CssdButtonsPanelLoader : public Ui::Loader {
	Q_OBJECT
public:
	CssdButtonsPanelLoader(QWidget *parent = nullptr);
	void loadNext();

	//void setSource(Source *src) = delete;
	//void setSourceAnimated(Source *src, AnimMode animMode = AnimMode::Fade, int pause = 0) = delete;
private:
	int _cur;
};

class CssdPage : public QWidget
{
    Q_OBJECT
public:
    explicit CssdPage(QWidget *parent = 0);
    ~CssdPage() = default;
private:
	using InternalLoader = Ui::CyclicLoader<PollutedAreaPanel, CleanAreaPanel, AsepsisAreaPanel, OperatingAreaPanel>;
	InternalLoader *_loader;
};
