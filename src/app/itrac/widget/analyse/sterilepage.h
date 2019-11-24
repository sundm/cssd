#pragma once

#include <QWidget>
#include <QtCharts/QChartGlobal>
#include "core/net/jsonhttpclient.h"

class FilterWidget;
class FilterGroup;
class Filter;
class QStandardItemModel;
class QTableView;
class QPushButton;
class QPaginationWidget;
class WaitingSpinner;

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
class QChart;
QT_CHARTS_END_NAMESPACE
QT_CHARTS_USE_NAMESPACE

class SterilePage : public QWidget, public JsonHttpClient
{
	Q_OBJECT

public:
	SterilePage(QWidget *parent);
	~SterilePage();

private slots:
	void refresh();

protected slots:
	void showFilterPopup(bool);

private:
	void onFilterChanged(Filter &);
	FilterGroup *createFilterGroup();
	void doSearch(int page = 1);
	QString toString(int v);

	QPushButton *_filterButton;
	QPaginationWidget *_paginator;
	QTableView *_view;
	FilterWidget *_filterWidget;
	Filter *_filter;
	QStandardItemModel * _model;
	WaitingSpinner *_waiter;

	QChartView *_chartView;
	QChartView *_barchartView;

	int _visibleCount;
};
