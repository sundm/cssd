#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "devicedao.h"
#include "errors.h"

result_t DeviceDao :: getWasherList(
	QList<Washer> *washers, bool onlyAvailable/* = true*/)
{
	return getDeviceList(Rt::Washer, washers, onlyAvailable);
}

result_t DeviceDao::getSterilizerList(QList<Sterilizer> *sterilizers, bool onlyAvailable/* = true*/)
{
	return getDeviceList(Rt::Sterilizer, sterilizers, onlyAvailable);
}

result_t DeviceDao::getAllDeivces(QList<Device> *devices, bool onlyAvailable /*= true */)
{
	return getDeviceList(Rt::UnknownDevice, devices, onlyAvailable);
}

result_t DeviceDao::getDeviceList(
	Rt::DeviceCategory cat,
	QList<Device> *devices,
	bool onlyAvailable/* = true*/)
{
	QString sql = "SELECT id, name, status, cycle_count, cycle_date,"
		" cycle_total, production_time, last_maintain_time, maintain_cycle, sterilize_type"
		" FROM t_device";
	bool hasWhere = false;

	if (Rt::UnknownDevice != cat) {
		sql.append(QString(" WHERE category = %1").arg(cat));
		hasWhere = true;
	}
	if (onlyAvailable) {
		sql.append(hasWhere ? " AND " : " WHERE ");
		sql.append(QString("status = %1").arg(Rt::Status::Normal));
	}

	QSqlQuery q;
	if (!q.exec(sql))
		return q.lastError().text();

	if (devices) {
		Device d;
		while (q.next()) {
			d.id = q.value(0).toInt();
			d.name = q.value(1).toString();
			d.category = Rt::DeviceCategory::Washer;
			d.status = static_cast<Rt::Status>(q.value(2).toInt());
			d.cycleCount = q.value(3).toInt();
			d.cycleDate = q.value(4).toDate();
			d.cycleTotal = q.value(5).toInt();
			d.productionDate = q.value(6).toDate();
			d.lastMaintainTime = q.value(7).toDateTime();
			d.maintainCycle = q.value(8).toUInt();
			d.sterilizeType = static_cast<Rt::SterilizeType>(q.value(9).toInt());
			devices->append(d);
		}
	}
	return 0;
}

