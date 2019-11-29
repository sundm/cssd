#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "flowdao.h"
#include "errors.h"
#include "../entity/package.h"
#include "../entity/operator.h"

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
	return 0;

	// update package status
	// update r_package
	sql = QString("UPDATE r_package SET status=%1").arg(Rt::Recycled) + whereClause;
	if (!q.exec(sql))
		return q.lastError().text();
	if (1 != q.numRowsAffected())
		qWarning("Internal db error: update r_package.status in addRecycle()");

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

result_t FlowDao::addWash(const QList<Package> &pkgs, const Operator& op)
{
	//if (pkgs.isEmpty()) return 0;

	return 0;
}
