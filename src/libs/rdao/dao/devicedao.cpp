#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "devicedao.h"
#include "daoutil.h"
#include "errors.h"

result_t DeviceDao::getDevice(int id, Device *d, bool withPrograms/* = false*/)
{
	QSqlQuery q;
	q.prepare("SELECT name, category, status,"
		" if(TO_DAYS(NOW())=TO_DAYS(cycle_date), cycle_count, 0) AS cycle_today,"
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
		d->cycleToday = q.value(3).toInt();
		d->cycleTotal = q.value(4).toInt();
		d->productionDate = q.value(5).toDate();
		d->lastMaintainTime = q.value(6).toDateTime();
		d->maintainCycle = q.value(7).toUInt();
		d->sterilizeType = static_cast<Rt::SterilizeType>(q.value(8).toInt());

		d->programs.clear();
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

result_t DeviceDao::stopDevice(int id)
{
	QSqlQuery q;
	q.prepare("UPDATE t_device SET status=? WHERE id = ? AND status=?");
	q.addBindValue(Rt::Idle);
	q.addBindValue(id);
	q.addBindValue(Rt::Running);
	if (!q.exec())
		return q.lastError().text();
	if (1 != q.numRowsAffected()) {
		qWarning("Internal error: update t_device in stopDevice()");
		return 0;
	}

	// update r_wash_batch
	q.prepare("UPDATE r_wash_batch a, t_device b"
		" SET a.finish_time=NOW()"
		" WHERE a.device_id = ? AND a.total_count=b.cycle_total"); // TODO: create a union KEY in DB?
	q.addBindValue(id);
	if (!q.exec())
		return q.lastError().text();
	if (1 != q.numRowsAffected())
		qWarning("Internal error: update r_wash_batch in stopDevice()");

	return 0;
}

result_t DeviceDao::getProgramsForDevice(int deviceId, QList<Program> *programs)
{
	QSqlQuery q;
	q.prepare("SELECT b.id, b.category, b.name, b.remark FROM"
		" t_device_programs a"
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
	QString sql = "SELECT id, name, category, status,"
		" if(TO_DAYS(NOW())=TO_DAYS(cycle_date), cycle_count, 0) AS cycle_today,"
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
			d.cycleToday = q.value(4).toInt();
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

result_t DeviceDao::startDevice(int id)
{
	QSqlQuery q;
	q.prepare("UPDATE t_device"
		" SET status=?, cycle_count=if(TO_DAYS(NOW())=TO_DAYS(cycle_date), cycle_count+1, 1),"
		" cycle_date=NOW(), cycle_total=cycle_total+1"
		" WHERE id = ?");
	q.addBindValue(Rt::Running);
	q.addBindValue(id);
	if (!q.exec())
		return q.lastError().text();
	if (1 != q.numRowsAffected())
		qWarning("Internal error: update t_device in startDevice()");

	return 0;
}
