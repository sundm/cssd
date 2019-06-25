#include "xtimescope.h"

XDateScope::XDateScope(QDate f, QDate t) {
	from = f;
	to = t;
}

XDateScope::XDateScope(XPeriod p)
{
	to = QDate::currentDate();
	switch (p)
	{
	case XPeriod::Today:
		from = to;
		break;
	case XPeriod::ThisWeek:
		from = to.addDays(1 - to.dayOfWeek());
		break;
	case XPeriod::ThisMonth:
		from = to.addDays(1 - to.day());
		break;
	case XPeriod::ThisSeason:
		from = QDate(to.year(), (to.month() - 1) / 3 * 3 + 1, 1);
		break;
	case XPeriod::ThisYear:
		from = QDate(to.year(), 1, 1);
		break;
	case XPeriod::RecentWeek:
		from = to.addDays(-7);
		break;
	default:
		from = to = QDate();
		break;
	}
}

