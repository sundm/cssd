#pragma once

#include <QWidget>

class QTabWidget;
class AnalysePage : public QWidget
{
	Q_OBJECT

public:
	enum {KPI, Device, Dept, Cost, RealTime};
	AnalysePage(QWidget *parent = Q_NULLPTR);

private slots:
	void showPage(int id);
	void closePage(int index);

private:
	QHash<int, QWidget*> _pageMap;
	QTabWidget *_tabWidget;
};
