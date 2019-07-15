#include <QHBoxLayout>
#include <QFrame>
#include "filtergroup.h"
#include "filterrow.h"

FilterGroup::FilterGroup(QWidget *parent)
	: QWidget(parent), _layout(new QVBoxLayout(this))
{
	//setWindowFlags(windowFlags() | Qt::SubWindow);
	//setAutoFillBackground(true);
	_layout->setContentsMargins(5, 0, 5, 0);
}

FilterGroup::~FilterGroup()
{
}

Filter FilterGroup::filter()
{
	Filter f;
	for(FilterRow *row: _rows) {
		row->setCondition2Filter(&f);
	}
	return f;
}

void FilterGroup::addRow(FilterRow *row)
{
	_layout->addWidget(row);
	_rows.append(row);
}

void FilterGroup::addSeperator()
{
	QFrame *line = new QFrame(this);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	_layout->addWidget(line);
}

void FilterGroup::reset()
{
	for(FilterRow *row: _rows) {
		row->reset();
	}
}


IssueFilterGroup::IssueFilterGroup(QWidget *parent)
	:FilterGroup(parent)
{
	dateRow = new DateFilterRow("日期", this);
	addRow(dateRow);
}
