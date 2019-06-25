#pragma once

#include <QWidget>

class QLayout;

namespace Qt {
	enum Orientation;
}

namespace Ui {

	void addSpacer(QLayout *layout, Qt::Orientation orient, int space = 40);

	void addPrimaryShortcut(QWidget *w, int key, const char *method);

	QWidget * createSeperator(Qt::Orientation orient);

} // namespace Ui

class Stylable : public QWidget {
	Q_OBJECT
public:
	Stylable(QWidget *parent = nullptr);
	virtual ~Stylable() = default;
protected:
	void paintEvent(QPaintEvent *e) override;
};

class BasicOverlay : public Stylable {
	Q_OBJECT
public:
	BasicOverlay(QWidget *parent = nullptr);
	~BasicOverlay() = default;
protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
};
