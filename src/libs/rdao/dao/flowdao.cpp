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
	// check the latest package flow status
	if (Rt::Recycled == pkg.status) // already recycled
		return "该包已回收，请勿重复操作";
		
	// get the department where the package was used (if any)
	int deptId = 0;
	QString deptName;
	if (pkg.status >= Rt::Dispatched) { // FIXME: should be Rt::Received?
		QString sql = QString("SELECT to_dept_id, to_dept_name FROM r_dispatch"
			" WHERE pkg_udi='%1' AND pkg_cycle=%2").arg(pkg.udi).arg(pkg.cycle);
		QSqlQuery q;
		if (!q.exec(sql))
			return q.lastError().text();
		if (q.first()) {
			deptId = q.value(0).toInt();
			deptName = q.value(1).toString();
		}
	}

	// start transaction
	QSqlDatabase db = QSqlDatabase::database();
	db.transaction();

	// add recycle
	QSqlQuery q;
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

	if (!q.exec()) {
		db.rollback();
		return q.lastError().text();
	}

	// update package status
	result_t res = updatePackageStatus(pkg, Rt::Recycled);
	if (!res.isOk()) {
		db.rollback();
		return res.msg();
	}

	db.commit();
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

	// add a new wash batch
	result_t res = addDeviceBatch(deviceId, program, pkgs, op);
	if (!res.isOk()) {
		return res;
	}

	// insert a new record for each package in r_package, since a loop always starts with washing.
	QString sql = "INSERT INTO r_package"
		" (pkg_udi, pkg_name, pkg_type_name, dept_name, pkg_cycle, pkg_type_id, dept_id) VALUES";
	QStringList values;
	for each (const Package &pkg in pkgs) {
		QString value = QString(" ('%1', '%2', '%3', '%4', %5, %6, %7)").
			arg(pkg.udi, pkg.name, pkg.name, pkg.dept.name).  // TODO, Package has no typename
			arg(pkg.cycle + 1).arg(pkg.typeId).arg(pkg.dept.id);
		values << value;
	}
	sql.append(values.join(','));

	QSqlQuery q;
	if (!q.exec(sql))
		return q.lastError().text();

	// update cycle/status for each package
	values.clear();
	for each (const Package &pkg in pkgs) {
		values << QString("'%1'").arg(pkg.udi);
	}
	sql = QString("UPDATE t_package SET cycle=cycle+1, status=%1 WHERE udi IN (%2)").
		arg(Rt::Washed).arg(values.join(','));
	if (!q.exec(sql))
		return q.lastError().text();

	// update instruments' cycle for each package
	sql = QString("UPDATE t_instrument SET cycle=cycle+1 WHERE pkg_udi IN (%1)").arg(values.join(','));
	if (!q.exec(sql))
		return q.lastError().text();

	return 0;
}

result_t FlowDao::addPack(const Package &pkg, const Operator& op, const Operator &checker, LabelInfo *li)
{
	QSqlQuery q;

	// check the latest package flow status
	if (Rt::Washed != pkg.status)
		return "该包未进入待配包序列，请确认该包已完成清洗流程";

	// update package status
	result_t res = updatePackageStatus(pkg, Rt::Packed);
	if (!res.isOk()) return res;

	// add pack
	// TODO, label id use MySQL autoincrement
	q.prepare(QString(
		"INSERT INTO r_pack (pkg_udi, pkg_cycle, pkg_name, dept_id,"
		" dept_name, op_id, op_name, pack_type_id, pack_type_name, expire_time, check_op_id, check_op_name)"
		" VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, DATE_ADD(NOW(), INTERVAL %1 DAY), ?, ?)"
		).arg(pkg.packType.validPeriod));
	q.addBindValue(pkg.udi);
	q.addBindValue(pkg.cycle);
	q.addBindValue(pkg.name);
	q.addBindValue(pkg.dept.id);
	q.addBindValue(pkg.dept.name);
	q.addBindValue(op.id);
	q.addBindValue(op.name);
	q.addBindValue(pkg.packType.id);
	q.addBindValue(pkg.packType.name);
	q.addBindValue(checker.id);
	q.addBindValue(checker.name);
	if (!q.exec())
		return q.lastError().text();

	// set label info
	if (li) {
		if (!q.exec("SELECT id, pack_time, expire_time FROM r_pack where id=LAST_INSERT_ID()"))
			return q.lastError().text();
		if (q.first()) {
			li->labelId = q.value(0).toString();
			li->packDate = q.value(1).toDate();
			li->expireDate = q.value(1).toDate();
		}
	}

	return 0;
}

result_t FlowDao::addSterilization(
	int deviceId,
	const Program &program,
	const QList<Package> &pkgs,
	const Operator &op)
{
	if (pkgs.isEmpty()) return 0;

	// add a new sterilization batch
	result_t res = addDeviceBatch(deviceId, program, pkgs, op);
	if (!res.isOk()) {
		return res;
	}

	// update status for each package
	res = updatePackageStatus(pkgs, Rt::Sterilized);
	if (!res.isOk()) {
		return res;
	}

	return 0;
}

result_t FlowDao::updateSterilizationResult(
	const QString &batchId,
	const Operator &op,
	const SterilizeResult &result)
{
	bool commitPhy = result.isPhyVerdictValid();
	bool commitChe = result.isCheVerdictValid();
	bool commitBio = result.isBioVerdictValid();

	if (commitPhy != commitChe)
		return "物理和化学监测结果必须同时提交";
	if (!commitPhy && !commitBio) // update nothing
		return 0;

	QSqlQuery q;
	if (!q.exec(QString("SELECT phy_check_result, che_check_result, bio_check_result"
		" FROM r_ster_batch WHERE batch_id=").append(batchId))) {
		return q.lastError().text();
	}
	if (!q.first()) {
		return "没有找到对应的灭菌批次信息";
	}
	Rt::SterilizeVerdict phyVerdict = static_cast<Rt::SterilizeVerdict>(q.value(0).toInt());
	Rt::SterilizeVerdict cheVerdict = static_cast<Rt::SterilizeVerdict>(q.value(1).toInt());
	Rt::SterilizeVerdict bioVerdict = static_cast<Rt::SterilizeVerdict>(q.value(2).toInt());
	bool phyChecked = (phyVerdict != Rt::Unchecked);
	bool bioChecked = (bioVerdict != Rt::Unchecked);
	if ((commitPhy && phyChecked) || (commitBio && bioChecked)) {
		return "请勿重复提交监测结果";
	}

	// start transaction
	QSqlDatabase db = QSqlDatabase::database();
	db.transaction();

	QSqlQuery tq;
	// update wet-pack info if any
	QString sql = "UPDATE r_ster_package SET wet_pack=1 WHERE (pkg_udi, pkg_cycle) IN ";
	QStringList wetPacks;
	for each (auto &pkg in result.packages) {
		if (pkg.isWetPack) {
			QString value = QString("('%1', %2)").arg(pkg.udi).arg(pkg.cycle);
			wetPacks << value;
		}
	}
	if (!wetPacks.isEmpty()) {
		sql.append("(").append(wetPacks.join(',')).append(")");
		if (!tq.exec(sql)) {
			db.rollback();
			return tq.lastError().text();
		}
	}

	// update verdicts
	sql = "update r_ster_batch SET";
	QString bioUpdate = " bio_check_result=?, bio_check_time=NOW(), bio_check_op_id=?, bio_check_op_name=?";
	if (commitPhy) {
		sql += " phy_check_result=?, phy_check_time=NOW(), phy_check_op_id=?, phy_check_op_name=?,"
			" che_check_result=?, che_check_time=NOW(), che_check_op_id=?, che_check_op_name=?";
		if (commitBio) {
			sql += "," + bioUpdate;
		}
	}
	else {
		sql += bioUpdate;
	}
	sql += ", has_wet_pack=?, has_label_off=? WHERE batch_id=?";

	tq.prepare(sql);
	if (commitPhy) {
		tq.addBindValue(result.phyVerdict);
		tq.addBindValue(op.id);
		tq.addBindValue(op.name);
		tq.addBindValue(result.cheVerdict);
		tq.addBindValue(op.id);
		tq.addBindValue(op.name);
	}
	if (commitBio) {
		tq.addBindValue(result.bioVerdict);
		tq.addBindValue(op.id);
		tq.addBindValue(op.name);
	}
	tq.addBindValue(!wetPacks.isEmpty());
	tq.addBindValue(result.hasLabelOff);
	tq.addBindValue(batchId);

	if (!tq.exec()) {
		db.rollback();
		return tq.lastError().text();
	}

	// update package status
	QList<Package> pkgs;
	result_t res = this->getPackagesInBatch(batchId, &pkgs);
	if (res.isOk()) {
		db.rollback();
		return res.msg();
	}
	
	res = updatePackageStatus(pkgs, Rt::SterilizeResultChecked);
	if (res.isOk()) {
		db.rollback();
		return res.msg();
	}

	db.commit();
	return 0;
}

result_t FlowDao::getDeviceBatchInfoByPackage(const Package &pkg, DeviceBatchInfo *dbi)
{
	QSqlQuery q;
	q.prepare("SELECT batch_id, device_name, program_name, op_name, cycle_count, cycle_total,"
		" start_time, finish_time, phy_check_result, che_check_result, bio_check_result"
		" FROM r_ster_batch"
		" WHERE batch_id = (SELECT batch_id FROM r_ster_package WHERE pkg_udi=? AND pkg_cycle=?)");
	q.addBindValue(pkg.udi);
	q.addBindValue(pkg.cycle);
	if (!q.exec())
		return q.lastError().text();
	if (!q.first())
		return "未找到该包对应的锅次信息";
	if (dbi) {
		dbi->batchId = q.value(0).toString();
		dbi->deviceName = q.value(1).toString();
		dbi->programName = q.value(2).toString(); 
		dbi->opName = q.value(3).toString();
		dbi->cycleCount = q.value(4).toUInt();
		dbi->cycleTotal = q.value(5).toUInt();
		dbi->startTime = q.value(6).toDateTime();
		dbi->finishTime = q.value(7).toDateTime();
		dbi->result.phyVerdict = static_cast<Rt::SterilizeVerdict>(q.value(8).toInt());
		dbi->result.cheVerdict = static_cast<Rt::SterilizeVerdict>(q.value(9).toInt());
		dbi->result.bioVerdict = static_cast<Rt::SterilizeVerdict>(q.value(10).toInt());
	}

	// get packages under the same batch
	q.prepare("SELECT pkg_udi, pkg_cycle, wet_pack, pkg_name FROM r_ster_package WHERE batch_id = ?");
	q.addBindValue(dbi->batchId);
	if (!q.exec())
		return q.lastError().text();
	while (q.next()) {
		dbi->result.packages.append(SterilizeResult::PackageItem(
			q.value(0).toString(), // udi
			q.value(1).toInt(), // cycle
			q.value(2).toBool(), // wetpack
			q.value(3).toString() // name
		));
	}

	return 0;
}

result_t FlowDao::addDispatch(const QList<Package> &pkgs, const Department &dept, const Operator &op)
{
	// start transaction
	QSqlDatabase db = QSqlDatabase::database();
	db.transaction();

	// add dispatch
	QSqlQuery q;
	QString sql = "INSERT INTO r_dispatch (pkg_udi, pkg_name, to_dept_name, op_name,"
		" pkg_cycle, to_dept_id, op_id) VALUES";
	QStringList values;
	for each (const Package &pkg in pkgs) {
		QString value = QString(" ('%1', '%2', '%3', '%4', %5, %6, %7)").
			arg(pkg.udi, pkg.name, dept.name, op.name).
			arg(pkg.cycle).arg(dept.id).arg(op.id);
		values << value;
	}
	sql.append(values.join(','));
	if (!q.exec(sql)) {
		db.rollback();
		return q.lastError().text();
	}

	// update package status
	result_t res = updatePackageStatus(pkgs, Rt::Dispatched);
	if (!res.isOk()) {
		db.rollback();
		return res.msg();
	}

	db.commit();
	return 0;
}

result_t FlowDao::addSurgeryPreCheck(const Surgery &surgery, const Operator &op)
{
	return 0;
}

result_t FlowDao::addSurgeryPostCheck(int surgeryId, const Operator &op)
{
	return 0;
}

/**
 * update status for a single package in table `r_package` and `t_package `
 */
result_t FlowDao::updatePackageStatus(const Package &pkg, Rt::FlowStatus fs)
{
	QSqlQuery q;

	// update r_package
	if (Rt::UnknownFlowStatus != pkg.status) { // a new package is not in flow control
		QString sql = QString("UPDATE r_package SET status=%1"
			" WHERE pkg_udi='%2' AND pkg_cycle=%3").arg(fs).arg(pkg.udi).arg(pkg.cycle);
		if (!q.exec(sql))
			return q.lastError().text();
		if (1 != q.numRowsAffected())
			qWarning("Internal db error: update r_package.status");
	}

	// update t_package
	q.prepare("UPDATE t_package SET status=? WHERE udi=?");
	q.addBindValue(fs);
	q.addBindValue(pkg.udi);
	if (!q.exec())
		return q.lastError().text();
	if (1 != q.numRowsAffected())
		qWarning("Internal db error: update t_package.status");

	return 0;
}

/**
 * update status for a bunch of packages in table `r_package` and `t_package `,
 * do not call this for washing, since washing has to do extra things on package/instrument cycles.
 */
result_t FlowDao::updatePackageStatus(const QList<Package> &pkgs, Rt::FlowStatus fs)
{
	QSqlQuery q;

	// update status in `r_package`
	QString sql = QString("UPDATE r_package SET status=%1 WHERE (pkg_udi, pkg_cycle) IN ").arg(fs);
	QStringList values;
	for each (const Package &pkg in pkgs) {
		QString value = QString("('%1', %2)").arg(pkg.udi).arg(pkg.cycle);
		values << value;
	}
	sql.append("(").append(values.join(',')).append(")");
	if (!q.exec(sql))
		return q.lastError().text();

	// update status for each package in `t_package`
	values.clear();
	for each (const Package &pkg in pkgs) {
		values << QString("'%1'").arg(pkg.udi);
	}
	sql = QString("UPDATE t_package SET status=%1 WHERE udi IN (%2)").
		arg(fs).arg(values.join(','));
	if (!q.exec(sql))
		return q.lastError().text();

	return 0;
}

result_t FlowDao::addDeviceBatch(
	int deviceId,
	const Program &program,
	const QList<Package> &pkgs,
	const Operator &op)
{
	// start the device
	DeviceDao dao;
	result_t res = dao.startDevice(deviceId);
	if (!res.isOk()) {
		return res;
	}

	// get the device
	Device device;
	res = dao.getDevice(deviceId, &device);
	if (!res.isOk()) {
		return res;
	}

	// TODO: check the device category passed in?
	bool forWash = (Rt::DeviceCategory::Washer == device.category);
	QString insertHeader("INSERT INTO ");

	// insert a batch
	QSqlQuery q;
	q.prepare(insertHeader + (forWash ? "r_wash_batch" : "r_ster_batch") +
		" (batch_id, device_id, device_name, program_id,"
		" program_name, cycle_count, total_count, start_time, op_id, op_name) VALUES"
		" (?, ?, ?, ?, ?, ?, ?, now(), ?, ?)");
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

	// insert packages for this batch
	QString sql = insertHeader + (forWash ? "r_wash_package" : "r_ster_package") +
		" (batch_id, pkg_udi, pkg_name, pkg_cycle) VALUES";
	QStringList values;
	for each (const Package &pkg in pkgs) {
		QString value = QString(" ('%1', '%2', '%3', %4)").
			arg(batchId, pkg.udi, pkg.name).arg(forWash ? pkg.cycle + 1 : pkg.cycle);
		values << value;
	}
	sql.append(values.join(','));
	if (!q.exec(sql))
		return q.lastError().text();

	return 0;
}

result_t FlowDao::getPackagesInBatch(const QString &batchId, QList<Package> *pkgs)
{
	QSqlQuery q;
	q.prepare("SELECT pkg_udi, pkg_cycle FROM r_ster_package WHERE batch_id=?");
	q.addBindValue(batchId);
	if (!q.exec()) {
		return q.lastError().text();
	}

	if (pkgs) {
		Package pkg;
		while (q.next()) {
			pkg.udi = q.value(0).toString();
			pkg.cycle = q.value(1).toInt();
			pkgs->append(pkg);
		}
	}
	return 0;
}

