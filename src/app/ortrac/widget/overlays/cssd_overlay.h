#pragma once

#include "ui/ui_commons.h"
#include <qscanner/qscanner.h>

class QVBoxLayout;
class QPropertyAnimation;


class CssdOverlayPanel : public QWidget, public Scanable {
	Q_OBJECT
public:
	CssdOverlayPanel(QWidget *parent = nullptr) :QWidget(parent), Scanable() {};
	virtual bool accept() { return true; };
};

class CssdOverlay : public BasicOverlay
{
	Q_OBJECT
public:
	CssdOverlay(const QString &title, QWidget *parent = nullptr);
	void setCentralPanel(CssdOverlayPanel *);

	virtual void showAnimated();

public slots:
	void cancel();

private:
	friend class CssdOverlayPanel;

	void setChildrenVisible(bool);
	void closeAnimated();

	QVBoxLayout * _layout;
	QPropertyAnimation *_anim;
	CssdOverlayPanel *_panel;
};
