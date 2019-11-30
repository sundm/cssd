#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "flowdao.h"
#include "errors.h"
#include "daoutil.h"
#include "devicedao.h"
#include "../entity/package.h"
#include "../entity/operator.h"
#include "../entity/device.h"

result_t FlowDao::addRecycle(const Package &pkg, const Operator& op)
{
	QSqlQuery q;
	QString sql;
	QString whereClause = QString(" WHERE pkg_udi='%1' AND pkg_cycle=%2").arg(pkg.udi).arg(pkg.cycle);

	// check the latest package flow status
	if (Rt::Recycled == pkg.status) // already recycled
		return "该包已回收，请勿重复操作";
		
	// get the department where the package was used (if any)
	int deptId = 0;
	QString deptName;
	if (pkg.status >= Rt::Dispatched) { // FIXME: should be Rt::Received?
		sql = "SELECT to_dept_id, to_dept_name FROM r_dispatch" + whereClause;
		if (!q.exec(sql))
			return q.lastError().text();
		if (q.first()) {
			deptId = q.value(0).toInt();
			deptName = q.value(1).toString();
		}
	}

	// add recycle
	q.prepare("INSERT INTO r_recycle (pkg_udi, pkg_cycle, pkg_name, from_dept_id,"
		" from_dept_name, op_id, op_name)"
		" VALUES (?, ?, ?, ?, ?, ?, ?)");
	q.addBindValue(pkg.udi);
	q.addBindValue(pkg.cycle);
	q.addBindValue(pkg.name);
	q.addBindValue(deptId);
	q.addBindValue(deptId ? deptName : QVariant(QVariant::String));
	q.addBindValue(op.id);
	q.addBindValue(op.name);

	if (!q.exec())
		return q.lastError().text();

	// update r_package
	if (Rt::UnknownFlowStatus != pkg.status) { 
		// the package is in a flow, update status
		sql = QString("UPDATE r_package SET status=%1").arg(Rt::Recycled) + whereClause;
		if (!q.exec(sql))
			return q.lastError().text();
		if (1 != q.numRowsAffected())
			qWarning("Internal db error: update r_package.status in addRecycle()");
	}

	// update t_package
	q.prepare("UPDATE t_package SET status=? WHERE udi=?");
	q.addBindValue(Rt::Recycled);
	q.addBindValue(pkg.udi);
	if (!q.exec())
		return q.lastError().text();
	if (1 != q.numRowsAffected())
		qWarning("Internal db error: update t_package.status in addRecycle()");
	return 0;
}

result_t FlowDao::addWash(
	int deviceId,
	const Program &program,
	const QList<Package> &pkgs,
	const Operator &op)
{
	if (pkgs.isEmpty()) return 0;

	// TODO: check package status is Recycled or leave it to the app?

	// start the device
	DeviceDao dao;
	result_t res = dao.startDevice(deviceId);
	if (!res.isOk()) {
		return res;
	}

	Device device;
	res = dao.getDevice(deviceId, &device);
	if (!res.isOk()) {
		return res;
	}

	// insert a wash batch
	QSqlQuery q;
	q.prepare("INSERT INTO r_wash_batch (batch_id, device_id, device_name, program_id,"
		" program_name, cycle_count, total_count, start_time, op_id, op_name) VALUES"
		" (?, ?, ?, ?, ?, ?, ?, now(), ?, ?");
	QString batchId = DaoUtil::deviceBatchId(device.id, device.cycleTotal);
	q.addBindValue(batchId);
	q.addBindValue(device.id);
	q.addBindValue(device.name);
	q.addBindValue(program.id);
	q.addBindValue(program.name);
	q.addBindValue(device.cycleToday);
	q.addBindValue(device.cycleTotal);
	q.addBindValue(op.id);
	q.addBindValue(op.name);

	if (!q.exec())
		return q.lastError().text();
	if (1 != q.numRowsAffected())
		qWarning("Internal error: insert t_device in addWash()");

	// insert r_wash_package
	QString sql = "INSERT INTO r_wash_package"
		" (batch_id, pkg_udi, pkg_name, pkg_cycle) VALUES";
	QStringList values;
	for each (const Package &pkg in pkgs) {
		QString value = QString(" ('%1', '%2', '%3', %4)").
			arg(batchId, pkg.udi, pkg.name).arg(pkg.cycle + 1);
		values << value;
	}
	sql.append(values.join(','));
	if (!q.exec(sql))
		return q.lastError().text();

	// insert a new record for each package in r_package, since a loop always starts with washing.
	sql = "INSERT INTO r_package"
		" (pkg_udi, pkg_name, pkg_type_name, dept_name, pkg_cycle, pkg_type_id, dept_id) VALUES";
	values.clear();
	for each (const Package &pkg in pkgs) {
		QString value = QString(" ('%1', '%2', '%3', '%4', %5, %6, %7)").
			arg(pkg.udi, pkg.name, pkg.name, pkg.dept.name).  // TODO, Package has no typename
			arg(pkg.cycle + 1).arg(pkg.typeId).arg(pkg.dept.id);
		values << value;
	}
	sql.append(values.join(','));
	if (!q.exec(sql))
		return q.lastError().text();

	// update cycle/status for each package
	sql = "UPDATE t_package SET cycle=cycle+1, status=%1 WHERE udi IN (%2)";
	values.clear();
	for each (const Package &pkg in pkgs) {
		values << QString("'%1'").arg(pkg.udi);
	}
	sql.arg(Rt::Washed).arg(values.join(','));
	if (!q.exec(sql))
		return q.lastError().text();

	// update instruments' cycle for each package
	sql = QString("UPDATE t_instrument SET cycle=cycle+1 WHERE pkg_udi IN (%1)").arg(values.join(','));
	if (!q.exec(sql))
		return q.lastError().text();

	return 0;
}

