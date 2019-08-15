#pragma once

#include <QWidget>
#include <QtCharts>
#include "core/net/jsonhttpclient.h"

class FilterWidget;
class FilterGroup;
class Filter;
class QStandardItemModel;
class QTableView;
class QPushButton;
class QPaginationWidget;
class WaitingSpinner;

class WashPage : public QWidget, public JsonHttpClient
{
	Q_OBJECT

public:
	WashPage(QWidget *parent);
	~WashPage();

private slots:
	void refresh();

protected slots:
	void showFilterPopup(bool);

private:
	void onFilterChanged(Filter &);
	FilterGroup *createFilterGroup();
	void doSearch(int page = 1);

	QPushButton *_filterButton;
	QPaginationWidget *_paginator;
	QTableView *_view;
	FilterWidget *_filterWidget;
	Filter *_filter;
	QStandardItemModel * _model;
	WaitingSpinner *_waiter;

	QChartView *_chartView;

	int _visibleCount;
};
