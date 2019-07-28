#pragma once

#include <QWidget>
#include "core/net/jsonhttpclient.h"
#include <QtCharts/QChartGlobal>

class QComboBox;
class WaitingSpinner;
class DeptEdit;

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class DeptStatisticsPage : public QWidget
{
	Q_OBJECT

public:
	DeptStatisticsPage(QWidget *parent);
	~DeptStatisticsPage();

private slots:
	void doSearch();

private:
	void updateChart(const QList<QVariant> &data);

	QComboBox * _yearCombo;
	QComboBox * _monthCombo;
	DeptEdit *_deptEdit;
	QChartView *_view;
	QChart *_chart;
	WaitingSpinner *_waiter;
	JsonHttpClient _http;
};
