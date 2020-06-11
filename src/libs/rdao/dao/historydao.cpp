#include <QVariant>
#include <QSqlError>
#include <QDate>
#include "debugsqlquery.h"
#include "historydao.h"
#include "errors.h"

result_t HistoryDao::getRecycleHistoryList(const QDate &from_date, const QDate &to_date,
	const int op_id, const int detp_id, QList<RecycleHistory> *recycleHis,
	int *total,
	int page,
	int count)
{
	if (!recycleHis) return 0;
	recycleHis->clear();

	bool paginated = nullptr != total;

	QString sql = "SELECT pkg_udi, pkg_cycle, pkg_name, from_dept_name, op_name, recycle_time"
		" FROM r_recycle WHERE ";

	if (op_id > 0)
	{
		sql.append(" op_id = %1 AND").arg(op_id);
	}

	if (detp_id > 0)
	{
		sql.append(" from_dept_id = %1 AND").arg(detp_id);
	}

	sql.append(QString("date(recycle_time) BETWEEN '%1' AND '%2' ").arg(from_date.toString("yyyy-MM-dd")).arg(to_date.toString("yyyy-MM-dd")));

	if (paginated) { // do pagination
		count = qMax(20, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2;SELECT COUNT(id) FROM r_recycle").arg((page - 1)*count).arg(count));
	}

	QSqlQuery query;

	if (!query.exec(sql))
		return query.lastError().text();

	RecycleHistory recycle;
	while (query.next()) {
		recycle.pkg_udi = query.value(0).toString();
		recycle.pkg_cycle = query.value(1).toInt();
		recycle.pkg_name = query.value(2).toString();
		recycle.from_dept_name = query.value(3).toString();
		recycle.op_name = query.value(4).toString();
		recycle.op_time = query.value(5).toDateTime().toString("yyyy-MM-dd HH:MM:ss");

		recycleHis->append(recycle);
	}

	if (paginated) {
		if (!query.nextResult() || !query.first())
			return "Could not determine the total number";
		*total = query.value(0).toInt();
	}

	return 0;
}

result_t HistoryDao::getWashHistoryList(const QDate &from_date, const QDate &to_date,
	const int op_id, const int device_id, QList<WashHistory> *washHis,
	int *total,
	int page,
	int count)
{
	if (!washHis) return 0;
	washHis->clear();

	bool paginated = nullptr != total;

	QString sql = "SELECT device_name, cycle_count, cycle_total, program_name, start_time, op_name, check_op_name, check_result "
		" FROM r_wash_batch WHERE ";

	if (op_id > 0)
	{
		sql.append(" op_id = %1 AND").arg(op_id);
	}

	if (device_id > 0)
	{
		sql.append(" device_id = %1 AND").arg(device_id);
	}

	sql.append(QString("date(start_time) BETWEEN '%1' AND '%2' ").arg(from_date.toString("yyyy-MM-dd")).arg(to_date.toString("yyyy-MM-dd")));

	if (paginated) { // do pagination
		count = qMax(20, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2;SELECT COUNT(id) FROM r_wash_batch").arg((page - 1)*count).arg(count));
	}

	QSqlQuery query;

	if (!query.exec(sql))
		return query.lastError().text();

	WashHistory wash;
	while (query.next()) {
		wash.device_name = query.value(0).toString();
		wash.cycle_count = query.value(1).toInt();
		wash.cycle_total = query.value(2).toInt();
		wash.program_name = query.value(3).toString();
		wash.op_time = query.value(4).toDateTime().toString("yyyy-MM-dd HH:MM:ss");
		wash.op_name = query.value(5).toString();
		wash.check_op_name = query.value(6).toString();
		wash.check_result = query.value(7).toInt() == 1 ? QString("合格") : QString("不合格");
		
		washHis->append(wash);
	}

	if (paginated) {
		if (!query.nextResult() || !query.first())
			return "Could not determine the total number";
		*total = query.value(0).toInt();
	}

	return 0;
}

result_t HistoryDao::getPackHistoryList(const QDate &from_date, const QDate &to_date,
	const int op_id, const int pack_type_id, QList<PackHistory> *packHis,
	int *total,
	int page,
	int count)
{
	if (!packHis) return 0;
	packHis->clear();

	bool paginated = nullptr != total;

	QString sql = "SELECT pkg_udi, pkg_name, pack_time, pack_type_name, op_name, check_op_name "
		" FROM r_pack WHERE ";

	if (op_id > 0)
	{
		sql.append(" op_id = %1 AND").arg(op_id);
	}

	if (pack_type_id > 0)
	{
		sql.append(" pack_type_id = %1 AND").arg(pack_type_id);
	}

	sql.append(QString("date(pack_time) BETWEEN '%1' AND '%2' ").arg(from_date.toString("yyyy-MM-dd")).arg(to_date.toString("yyyy-MM-dd")));

	if (paginated) { // do pagination
		count = qMax(20, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2;SELECT COUNT(id) FROM r_pack").arg((page - 1)*count).arg(count));
	}

	QSqlQuery query;

	if (!query.exec(sql))
		return query.lastError().text();

	PackHistory pack;
	while (query.next()) {
		pack.pkg_udi = query.value(0).toString();
		pack.pkg_name = query.value(1).toString();
		pack.op_time = query.value(2).toDateTime().toString("yyyy-MM-dd HH:MM:ss");
		pack.packType = query.value(3).toString();
		pack.op_name = query.value(4).toString();
		pack.check_op_name = query.value(5).toString();

		packHis->append(pack);
	}

	if (paginated) {
		if (!query.nextResult() || !query.first())
			return "Could not determine the total number";
		*total = query.value(0).toInt();
	}

	return 0;
}

result_t HistoryDao::getSterHistoryList(const QDate &from_date, const QDate &to_date,
	const int op_id, const int device_id, QList<SterHistory> *sterHis,
	int *total,
	int page,
	int count)
{
	if (!sterHis) return 0;
	sterHis->clear();

	bool paginated = nullptr != total;

	QString sql = "SELECT device_name, cycle_count, cycle_total, program_name, start_time, op_name,"
		" phy_check_op_name, phy_check_time, phy_check_result,"
		" che_check_op_name, che_check_time, che_check_result,"
		" bio_check_op_name, bio_check_time, bio_check_result"
		" FROM r_ster_batch WHERE ";

	if (op_id > 0)
	{
		sql.append(" op_id = %1 AND").arg(op_id);
	}

	if (device_id > 0)
	{
		sql.append(" device_id = %1 AND").arg(device_id);
	}

	sql.append(QString("date(start_time) BETWEEN '%1' AND '%2' ").arg(from_date.toString("yyyy-MM-dd")).arg(to_date.toString("yyyy-MM-dd")));

	if (paginated) { // do pagination
		count = qMax(20, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2;SELECT COUNT(id) FROM r_ster_batch").arg((page - 1)*count).arg(count));
	}

	QSqlQuery query;

	if (!query.exec(sql))
		return query.lastError().text();

	SterHistory ster;
	while (query.next()) {
		ster.device_name = query.value(0).toString();
		ster.cycle_count = query.value(1).toInt();
		ster.cycle_total = query.value(2).toInt();
		ster.program_name = query.value(3).toString();
		ster.op_time = query.value(4).toDateTime().toString("yyyy-MM-dd HH:MM:ss");
		ster.op_name = query.value(5).toString();

		ster.phy_check_op_name = query.value(6).toString();
		ster.phy_check_time = query.value(7).toDateTime().toString("yyyy-MM-dd HH:MM:ss");
		ster.phy_check_result = query.value(8).toInt();

		ster.che_check_op_name = query.value(9).toString();
		ster.che_check_time = query.value(10).toDateTime().toString("yyyy-MM-dd HH:MM:ss");
		ster.che_check_result = query.value(11).toInt();

		ster.bio_check_op_name = query.value(12).toString();
		ster.bio_check_time = query.value(13).toDateTime().toString("yyyy-MM-dd HH:MM:ss");
		ster.bio_check_result = query.value(14).toInt();

		sterHis->append(ster);
	}

	if (paginated) {
		if (!query.nextResult() || !query.first())
			return "Could not determine the total number";
		*total = query.value(0).toInt();
	}

	return 0;
}

result_t HistoryDao::getDispatchHistoryList(const QDate &from_date, const QDate &to_date,
	const int op_id, const int detp_id, QList<DispatchHistory> *dispatchHis,
	int *total,
	int page,
	int count)
{
	if (!dispatchHis) return 0;
	dispatchHis->clear();

	bool paginated = nullptr != total;

	QString sql = "SELECT pkg_udi, pkg_name, to_dept_name, op_name, dispatch_time"
		" FROM r_dispatch WHERE ";

	if (op_id > 0)
	{
		sql.append(" op_id = %1 AND").arg(op_id);
	}

	if (detp_id > 0)
	{
		sql.append(" to_dept_id = %1 AND").arg(detp_id);
	}

	sql.append(QString("date(dispatch_time) BETWEEN '%1' AND '%2' ").arg(from_date.toString("yyyy-MM-dd")).arg(to_date.toString("yyyy-MM-dd")));

	if (paginated) { // do pagination
		count = qMax(20, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2;SELECT COUNT(id) FROM r_dispatch").arg((page - 1)*count).arg(count));
	}

	QSqlQuery query;

	if (!query.exec(sql))
		return query.lastError().text();

	DispatchHistory dispatch;
	while (query.next()) {
		dispatch.pkg_udi = query.value(0).toString();
		dispatch.pkg_name = query.value(1).toString();
		dispatch.dept_name = query.value(3).toString();
		dispatch.op_name = query.value(4).toString();
		dispatch.op_time = query.value(5).toDateTime().toString("yyyy-MM-dd HH:MM:ss");

		dispatchHis->append(dispatch);
	}

	if (paginated) {
		if (!query.nextResult() || !query.first())
			return "Could not determine the total number";
		*total = query.value(0).toInt();
	}

	return 0;
}

result_t HistoryDao::getOperatorHistoryList(const int year, const int month, QList<OperatorHistory> *opHistories)
{
	if (!opHistories) return 0;
	opHistories->clear();

	QSqlQuery query;
	query.prepare("select id,name FROM t_operator");
	if (!query.exec())
		return kErrorDbUnreachable;

	while (query.next())
	{
		OperatorHistory opHis;
		opHis.id = query.value(0).toInt();
		opHis.name = query.value(1).toString();

		QSqlQuery q;
		q.prepare("select COUNT(*) FROM r_recycle where op_id = ? and Year(recycle_time) = ? and Month(recycle_time) = ?");
		q.addBindValue(opHis.id);
		q.addBindValue(year);
		q.addBindValue(month);

		if (!q.exec())
			return kErrorDbUnreachable;
		q.next();
		opHis.recycleCount = q.value(0).toInt();


		q.prepare("select COUNT(*) FROM r_wash_batch where op_id = ? and Year(start_time) = ? and Month(start_time) = ?");
		q.addBindValue(opHis.id);
		q.addBindValue(year);
		q.addBindValue(month);

		if (!q.exec())
			return kErrorDbUnreachable;
		q.next();
		opHis.washCount = q.value(0).toInt();

		q.prepare("select COUNT(*) FROM r_pack where op_id = ? and Year(pack_time) = ? and Month(pack_time) = ?");
		q.addBindValue(opHis.id);
		q.addBindValue(year);
		q.addBindValue(month);

		if (!q.exec())
			return kErrorDbUnreachable;
		q.next();
		opHis.packCount = q.value(0).toInt();

		q.prepare("select COUNT(*) FROM r_ster_batch where op_id = ? and Year(start_time) = ? and Month(start_time) = ?");
		q.addBindValue(opHis.id);
		q.addBindValue(year);
		q.addBindValue(month);

		if (!q.exec())
			return kErrorDbUnreachable;
		q.next();
		opHis.sterCount = q.value(0).toInt();

		q.prepare("select COUNT(*) FROM r_dispatch where op_id = ? and Year(dispatch_time) = ? and Month(dispatch_time) = ?");
		q.addBindValue(opHis.id);
		q.addBindValue(year);
		q.addBindValue(month);

		if (!q.exec())
			return kErrorDbUnreachable;
		q.next();
		opHis.dispatchCount = q.value(0).toInt();

		opHistories->append(opHis);
	}

	return 0;
}