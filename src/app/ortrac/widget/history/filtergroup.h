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

class IssueFilterGroup : public FilterGroup
{
	Q_OBJECT

public:
	IssueFilterGroup(QWidget *parent);

private:
	FilterRow * dateRow;
};

