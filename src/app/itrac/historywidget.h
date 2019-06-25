#pragma once

#include <QWidget>
#include "ui_historywidget.h"

class HistoryWidget : public QWidget, public Ui::HistoryWidget
{
	Q_OBJECT

public:
	HistoryWidget(QWidget *parent = Q_NULLPTR);
	~HistoryWidget();

private slots:
	void showPage(int id);
	void closePage(int index);

private:
	QHash<int, QWidget*> _pageMap;
};
