#pragma once

#include <QWidget>

class QComboBox;
class TableView;
class QStandardItemModel;
class KpiPage : public QWidget
{
	Q_OBJECT

public:
	KpiPage(QWidget *parent);
	~KpiPage();

private slots:
	void doSearch();

private:
	QComboBox * _yearCombo;
	QComboBox * _monthCombo;
	TableView * _view;
	QStandardItemModel *_model;
};
