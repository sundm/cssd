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


RecycleFilterGroup::RecycleFilterGroup(QWidget *parent)
	:FilterGroup(parent)
{
	dateRow = new DateFilterRow("日期", this);
	userRow = new UserFilterRow("回收人员", this, UserFilterRow::USER::Operator);
	deptRow = new DeptFilterRow("来源科室", this);
	addRow(dateRow);
	addSeperator();
	addRow(userRow);
	addSeperator();
	addRow(deptRow);
}

WashFilterGroup::WashFilterGroup(QWidget *parent)
	:FilterGroup(parent)
{
	dateRow = new DateFilterRow("日期", this);
	userRow = new UserFilterRow("清洗人", this, UserFilterRow::USER::Operator);
	deviceRow = new DeviceFilterRow("清洗机", this, itrac::DeviceType::Washer);
	cycleRow = new CycleFilterRow("当日锅次", this);
	addRow(dateRow);
	addSeperator();
	addRow(userRow);
	addSeperator();
	addRow(deviceRow);
	addSeperator();
	addRow(cycleRow);
	//addSeperator();
}

WashAbnormalFilterGroup::WashAbnormalFilterGroup(QWidget *parent)
	:FilterGroup(parent)
{
	dateRow = new DateFilterRow("日期", this);
	userRow = new UserFilterRow("清洗人", this, UserFilterRow::USER::Operator);
	deviceRow = new DeviceFilterRow("清洗机", this, itrac::DeviceType::Washer);
	addRow(dateRow);
	addSeperator();
	addRow(deviceRow);
	addSeperator();
	addRow(userRow);
	
}

SterileFilterGroup::SterileFilterGroup(QWidget *parent)
	:FilterGroup(parent)
{
	dateRow = new DateFilterRow("日期", this);
	userRow = new UserFilterRow("灭菌员", this, UserFilterRow::USER::Operator);
	deviceRow = new DeviceFilterRow("灭菌机", this, itrac::DeviceType::Sterilizer);
	cycleRow = new CycleFilterRow("当日锅次", this);
	checkRow = new CheckFilterRow("是否合格", this);

	addRow(dateRow);
	addSeperator();
	addRow(userRow);
	addSeperator();
	addRow(deviceRow);
	addSeperator();
	addRow(cycleRow);
	addSeperator();
	addRow(checkRow);
}

SterileAbnormalFilterGroup::SterileAbnormalFilterGroup(QWidget *parent)
	:FilterGroup(parent)
{
	dateRow = new DateFilterRow("日期", this);
	userRow = new UserFilterRow("灭菌员", this, UserFilterRow::USER::Operator);
	checkerRow = new UserFilterRow("审核员", this, UserFilterRow::USER::Operator);
	deviceRow = new DeviceFilterRow("灭菌机", this, itrac::DeviceType::Sterilizer);

	addRow(dateRow);
	addSeperator();
	addRow(deviceRow);
	addSeperator();
	addRow(userRow);
	addSeperator();
	addRow(checkerRow);
}

PackFilterGroup::PackFilterGroup(QWidget *parent)
	:FilterGroup(parent)
{
	dateRow = new DateFilterRow("日期", this);
	opRow = new UserFilterRow("打包员", this, UserFilterRow::USER::Operator);
	cpRow = new UserFilterRow("审核员", this, UserFilterRow::USER::Checker);
	typeRow = new PackTypeFilterRow("打包方式", this);
	addRow(dateRow);
	addSeperator();
	addRow(opRow);
	addSeperator();
	addRow(cpRow);
	addSeperator();
	addRow(typeRow);
	//addSeperator();
}

DispatchFilterGroup::DispatchFilterGroup(QWidget *parent)
	:FilterGroup(parent)
{
	dateRow = new DateFilterRow("日期", this);
	userRow = new UserFilterRow("发放人员", this, UserFilterRow::USER::Operator);
	deptRow = new DeptFilterRow("发放科室", this);
	addRow(dateRow);
	addSeperator();
	addRow(userRow);
	addSeperator();
	addRow(deptRow);
	//addSeperator();
}