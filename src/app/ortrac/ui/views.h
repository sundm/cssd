#pragma once

#include <QTableView>

class TableView : public QTableView
{
	Q_OBJECT

public:
	TableView(QWidget *parent = Q_NULLPTR);
	int findRow(int column, int role, const QVariant &value);
	QModelIndexList selectedRows() const;

public slots:
	void clear();
	void removeSeletedRows();
};

class PaginationView : public TableView
{
	Q_OBJECT

public:
	PaginationView(QWidget *parent = Q_NULLPTR) :TableView(parent) {}
	void setPageCount(int count) { _pageCount = count; }

protected:
	int _pageCount = 10;
};
