#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "devicedao.h"
#include "errors.h"

result_t DeviceDao::getWasherList(
	QList<Washer> *washers, int page/* = 1*/, int count/* = -1*/)
{
	QSqlQuery q;

	QString sql = "SELECT id, category, name, pinyin, photo, is_vip"
		" FROM t_instrument_type";

	//if (-1 != count) { // do pagination
	//	count = qMax(10, count);
	//	page = qMax(1, page);
	//	sql.append(QString(" LIMIT %1, %2").arg((page-1)*count).arg(count));
	//}

	//if (!q.exec(sql))
	//	return q.lastError().text();

	//if (insTypes) {
	//	InstrumentType it;
	//	while (q.next()) {
	//		it.typeId = q.value(0).toInt();
	//		it.category = static_cast<Rt::InstrumentCategory>(q.value(1).toInt());
	//		it.name = q.value(2).toString();
	//		it.pinyin = q.value(3).toString();
	//		it.photo = q.value(4).toString();
	//		it.isVip = q.value(5).toBool();
	//		insTypes->append(it);
	//	}
	//}
	return 0;
}

result_t DeviceDao::getSterilizerList(QList<Sterilizer> *sterilizers, int page /*= 1*/, int count /*= -1*/)
{
	return 0;
}

