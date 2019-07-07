#pragma once

#include <QWidget>
#include "ui_leftbar.h"

class QButtonGroup;
class LeftBar : public QFrame, public Ui::LeftBar
{
	Q_OBJECT

public:
	enum {UseState, HistoryState, TraceState};

	LeftBar(QWidget *parent = Q_NULLPTR);
	~LeftBar();

signals:
	void currentChanged(int);

private slots:
	void updateButtonGroupState(int id);

private:
	QButtonGroup * _btns;
};
