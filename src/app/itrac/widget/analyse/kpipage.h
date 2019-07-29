#pragma once

#include <QWidget>
#include "core/net/jsonhttpclient.h"

class QComboBox;
class TableView;
class QStandardItemModel;
class WaitingSpinner;

class KpiPage : public QWidget
{
	Q_OBJECT

public:
	KpiPage(QWidget *parent);
	~KpiPage();

private slots:
	void doSearch();

private:
	void queryUserJobs();

	QComboBox * _yearCombo;
	QComboBox * _monthCombo;
	TableView * _view;
	QStandardItemModel *_model;
	WaitingSpinner *_waiter;
	JsonHttpClient _http;
};
