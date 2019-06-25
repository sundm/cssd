#ifndef XTIMESCOPE_H
#define XTIMESCOPE_H

#include "xernel_global.h"
#include <QDate>

enum XPeriod : int {
	Today,
	ThisWeek,
	ThisMonth,
	ThisSeason,
	ThisYear,
	RecentWeek,
	Custom
};

struct XERNEL_EXPORT XDateScope
{
	QDate from;
	QDate to;

	XDateScope(QDate f, QDate t);
	XDateScope(XPeriod);

};

#endif // !XTIMESCOPE_H
