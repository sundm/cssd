#pragma once

#include <QTabWidget>

namespace Ui {
	class FlatEdit;
}

class TableView;
class PackageEdit;
class QStandardItemModel;
class QComboBox;
class QDateEdit;

class OrderPanel : public QWidget
{
	Q_OBJECT

public:
	OrderPanel(QWidget *parent = Q_NULLPTR);

private slots:
	void addEntry();
	void commit();

private:
	TableView *_orderView;
	PackageEdit *_pkgEdit;
	QStandardItemModel *_model;
};

class HistoryOrderPanel : public QWidget
{
	Q_OBJECT

public:
	HistoryOrderPanel(QWidget *parent = Q_NULLPTR);

private slots:
	void changeSearchMethod(int);
	void onPeriodChanged();
	void doSearch();

private:
	TableView * _orderView;
	QStandardItemModel *_model;
	QWidget *_timeWidget;
	QComboBox *_periodCombo;
	QDateEdit *_fromDateEdit;
	QDateEdit *_toDateEdit;
	Ui::FlatEdit *_orderEdit;
};

class ClinicPage : public QTabWidget
{
	Q_OBJECT

public:
	ClinicPage(QWidget *parent = Q_NULLPTR);
};
