#pragma once

#include <QWidget>
#include "filter.h"

class FilterRow;
class QVBoxLayout;
class FilterGroup : public QWidget
{
	Q_OBJECT

public:
	FilterGroup(QWidget *parent);
	virtual ~FilterGroup();

	Filter filter();

public slots:
	void reset();

protected:
	void addRow(FilterRow *row);
	void addSeperator();

private:
	QVBoxLayout * _layout;
	QList<FilterRow*> _rows;
};

class RecycleFilterGroup : public FilterGroup
{
	Q_OBJECT

public:
	RecycleFilterGroup(QWidget *parent);

private:
	FilterRow * dateRow;
	FilterRow * userRow;
	FilterRow * packageRow;
	FilterRow * deptRow;
};

class WashFilterGroup : public FilterGroup
{
	Q_OBJECT

public:
	WashFilterGroup(QWidget *parent);

private:
	FilterRow * dateRow;
	FilterRow * userRow;
	FilterRow * deviceRow;
	FilterRow * cycleRow;
};


class DispatchFilterGroup : public FilterGroup
{
	Q_OBJECT

public:
	DispatchFilterGroup(QWidget *parent);

private:
	FilterRow * dateRow;
	FilterRow * userRow;
	FilterRow * deptRow;
};