#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QPushButton;
class FilterGroup;
class Filter;
class FilterWidget : public QWidget
{
	Q_OBJECT

public:
	FilterWidget(QWidget *parent = nullptr);
	FilterWidget(FilterGroup *fg, QWidget *parent = nullptr);
	~FilterWidget();

	void setFilterGroup(FilterGroup *fg);

signals:
	void filterReady(Filter &);

protected:
	void paintEvent(QPaintEvent *event);

private slots:
	void doSearch();

private:
	QVBoxLayout * _layout;
	FilterGroup *_filterGroup;
	QPushButton *_resetButton;
};

#endif FILTERWIDGET_H
