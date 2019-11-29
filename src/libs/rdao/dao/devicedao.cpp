#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "devicedao.h"
#include "errors.h"

result_t DeviceDao::getDevice(int id, Device *d, bool withPrograms/* = false*/)
{
	QSqlQuery q;
	q.prepare("SELECT name, category, status, cycle_count, cycle_date,"
		" cycle_total, production_time, last_maintain_time, maintain_cycle, sterilize_type"
		" FROM t_device"
		" WHERE id = ?");
	q.addBindValue(id);
	if (!q.exec())
		return q.lastError().text();
	if (!q.first())
		return "未找到对应的设备信息";

	if (d) {
		d->id = id;
		d->name = q.value(0).toString();
		d->category = static_cast<Rt::DeviceCategory>(q.value(1).toInt());
		d->status = static_cast<Rt::DeviceStatus>(q.value(2).toInt());
		d->cycleCount = q.value(3).toInt();
		d->cycleDate = q.value(4).toDate();
		d->cycleTotal = q.value(5).toInt();
		d->productionDate = q.value(6).toDate();
		d->lastMaintainTime = q.value(7).toDateTime();
		d->maintainCycle = q.value(8).toUInt();
		d->sterilizeType = static_cast<Rt::SterilizeType>(q.value(9).toInt());

		if (withPrograms) // get bound programs
			return this->getProgramsForDevice(id, &d->programs);
	}

	return 0;
}

result_t DeviceDao :: getWasherList(
	QList<Washer> *washers, bool excludeForbidden/* = true*/)
{
	return getDeviceList(Rt::Washer, washers, excludeForbidden);
}

result_t DeviceDao::getSterilizerList(QList<Sterilizer> *sterilizers, bool excludeForbidden/* = true*/)
{
	return getDeviceList(Rt::Sterilizer, sterilizers, excludeForbidden);
}

result_t DeviceDao::getAllDeivces(QList<Device> *devices, bool excludeForbidden /*= true */)
{
	return getDeviceList(Rt::UnknownDeviceCategory, devices, excludeForbidden);
}

result_t DeviceDao::getProgramsForDevice(int deviceId, QList<Program> *programs)
{
	QSqlQuery q;
	q.prepare("SELECT b.id, b.category, b.name, b.remark FROM"
		" device_programs a"
		" LEFT JOIN t_program b ON a.program_id = b.id"
		" WHERE a.device_id = ?");
	q.addBindValue(deviceId);

	if (!q.exec())
		return q.lastError().text();

	if (programs) {
		Program p;
		while (q.next()) {
			p.id = q.value(0).toInt();
			p.category = static_cast<Rt::DeviceCategory>(q.value(1).toInt());
			p.name = q.value(2).toString();
			p.remark = q.value(3).toString();
			programs->append(p);
		}
	}
	return 0;
}

result_t DeviceDao::getWashPrograms(QList<Program> *programs)
{
	return getProgramList(Rt::Washer, programs);
}

result_t DeviceDao::getSterilizePrograms(QList<Program> *programs)
{
	return getProgramList(Rt::Sterilizer, programs);
}

result_t DeviceDao::getAllPrograms(QList<Program> *programs)
{
	return getProgramList(Rt::UnknownDeviceCategory, programs);
}

result_t DeviceDao::getDeviceList(
	Rt::DeviceCategory cat,
	QList<Device> *devices,
	bool excludeForbidden/* = true*/)
{
	QString sql = "SELECT id, name, category, status, cycle_count, cycle_date,"
		" cycle_total, production_time, last_maintain_time, maintain_cycle, sterilize_type"
		" FROM t_device";
	bool hasWhere = false;

	if (Rt::UnknownDeviceCategory != cat) {
		sql.append(QString(" WHERE category = %1").arg(cat));
		hasWhere = true;
	}
	if (excludeForbidden) {
		sql.append(hasWhere ? " AND " : " WHERE ");
		sql.append(QString("status <> %1").arg(Rt::DeviceStatus::Forbidden));
	}

	QSqlQuery q;
	if (!q.exec(sql))
		return q.lastError().text();

	if (devices) {
		Device d;
		while (q.next()) {
			d.id = q.value(0).toInt();
			d.name = q.value(1).toString();
			d.category = static_cast<Rt::DeviceCategory>(q.value(2).toInt());
			d.status = static_cast<Rt::DeviceStatus>(q.value(3).toInt());
			d.cycleCount = q.value(4).toInt();
			d.cycleDate = q.value(5).toDate();
			d.cycleTotal = q.value(6).toInt();
			d.productionDate = q.value(7).toDate();
			d.lastMaintainTime = q.value(8).toDateTime();
			d.maintainCycle = q.value(9).toUInt();
			d.sterilizeType = static_cast<Rt::SterilizeType>(q.value(10).toInt());
			devices->append(d);
		}
	}
	return 0;
}

result_t DeviceDao::getProgramList(Rt::DeviceCategory cat, QList<Program> *programs)
{
	QString sql = "SELECT id, category, name, remark FROM t_program";

	if (Rt::UnknownDeviceCategory != cat) {
		sql.append(QString(" WHERE category = %1").arg(cat));
	}
	
	QSqlQuery q;
	if (!q.exec(sql))
		return q.lastError().text();

	if (programs) {
		Program p;
		while (q.next()) {
			p.id = q.value(0).toInt();
			p.category = static_cast<Rt::DeviceCategory>(q.value(1).toInt());
			p.name = q.value(2).toString();
			p.remark = q.value(3).toString();
			programs->append(p);
		}
	}
	return 0;
}

