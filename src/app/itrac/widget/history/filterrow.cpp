#include <QHBoxLayout>
#include <QDateEdit>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpacerItem>
#include <QButtonGroup>
#include <xernel/xtimescope.h>
#include "filterrow.h"
#include "filter.h"

FilterRow::FilterRow(const QString &head, QWidget *parent)
	: QWidget(parent), _layout(new QHBoxLayout)
{
	_layout->addWidget(new QLabel(head));
	_layout->addStretch();
}

FilterRow::~FilterRow()
{
}

DateFilterRow::DateFilterRow(const QString &head, QWidget *parent)
	:FilterRow(head, parent)
{
	_todayButton = new QRadioButton("今天", this);
	_weekButton = new QRadioButton("本周", this);
	_monthButton = new QRadioButton("本月", this);
	_yearButton = new QRadioButton("今年", this);
	_customButton = new QRadioButton("自定义", this);

	// add radio buttons into group
	_dateButtons = new QButtonGroup(this);
	_dateButtons->addButton(_todayButton, XPeriod::Today);
	_dateButtons->addButton(_weekButton, XPeriod::ThisWeek);
	_dateButtons->addButton(_monthButton, XPeriod::ThisMonth);
	_dateButtons->addButton(_yearButton, XPeriod::ThisYear);
	_dateButtons->addButton(_customButton, XPeriod::Custom);
	connect(_dateButtons, QOverload<int, bool>::of(&QButtonGroup::buttonToggled),
		this, &DateFilterRow::onDateButtonToggled);

	_layout->addWidget(_todayButton);
	_layout->addWidget(_weekButton);
	_layout->addWidget(_monthButton);
	_layout->addWidget(_yearButton);
	_layout->addWidget(_customButton);

	QDate today = QDate::currentDate();
	_startDateEdit = new QDateEdit(today, this);
	_startDateEdit->setCalendarPopup(true);
	_startDateEdit->setMaximumDate(today);
	_startDateEdit->setEnabled(false);
	_layout->addWidget(_startDateEdit);

	_layout->addWidget(new QLabel("-", this));

	_endDateEdit = new QDateEdit(today, this);
	_endDateEdit->setCalendarPopup(true);
	_endDateEdit->setMaximumDate(today);
	_endDateEdit->setEnabled(false);
	_layout->addWidget(_endDateEdit);

	_layout->addStretch(1);
	_layout->setSpacing(13);
	setLayout(_layout);

	_todayButton->setChecked(true);
}

void DateFilterRow::reset()
{
	_todayButton->setChecked(true);
}

void DateFilterRow::setCondition2Filter(Filter *f)
{
	f->setCondition(FilterFlag::StartDate, _startDateEdit->date());
	f->setCondition(FilterFlag::EndDate, _endDateEdit->date());
}

void DateFilterRow::onDateButtonToggled(int id, bool checked)
{
	if (XPeriod::Custom == id) {
		_startDateEdit->setEnabled(checked);
		_endDateEdit->setEnabled(checked);
		return;
	}

	// for other buttons, ignore their unchecked signals.
	if (!checked) return;

	XDateScope ds(static_cast<XPeriod>(id));
	_startDateEdit->setDate(ds.from);
	_endDateEdit->setDate(ds.to);
}

UserFilterRow::UserFilterRow(const QString &head, QWidget *parent)
	:FilterRow(head, parent)
{
	_userEdit = new QLineEdit(this);
	_layout->addWidget(_userEdit);

	_layout->addStretch(1);
	setLayout(_layout);
}

void UserFilterRow::reset()
{

}

void UserFilterRow::setCondition2Filter(Filter *f)
{

}

DeptFilterRow::DeptFilterRow(const QString &head, QWidget *parent)
	:FilterRow(head, parent)
{
	_deptEdit = new QLineEdit(this);
	_layout->addWidget(_deptEdit);

	_layout->addStretch(1);
	setLayout(_layout);
}

void DeptFilterRow::reset()
{

}

void DeptFilterRow::setCondition2Filter(Filter *f)
{

}
