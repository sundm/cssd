#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "tracedao.h"
#include "errors.h"
#include "surgerydao.h"
#include "../entity/trace.h"

namespace Internal {

	result_t getWashInfo(const QString & udi, int cycle, PackageFlow::Wash &wash)
	{
		QSqlQuery q;
		q.prepare("SELECT batch_id, device_name, program_name,"
			" cycle_count, cycle_total, start_time, finish_time, op_name"
			" FROM r_wash_batch"
			" WHERE batch_id = (SELECT batch_id FROM r_wash_package WHERE pkg_udi = ? AND pkg_cycle = ?)");
		q.addBindValue(udi);
		q.addBindValue(cycle);
		if (!q.exec())
			return kErrorDbUnreachable;
		if (!q.first())
			return "无此包清洗记录";

		wash.batchId = q.value(0).toString();
		wash.device = q.value(1).toString();
		wash.program = q.value(2).toString();
		wash.cycleCount = q.value(3).toInt();
		wash.cycleTotal = q.value(4).toInt();
		wash.startTime = q.value(5).toDateTime();
		wash.finishTime = q.value(6).toDateTime();
		wash.op = q.value(7).toString();

		return 0;
	}

	result_t getPackInfo(const QString & udi, int cycle, PackageFlow::Pack &pack)
	{
		QSqlQuery q;
		q.prepare("SELECT op_name, pack_time, check_op_name"
			" FROM r_pack"
			" WHERE pkg_udi = ? AND pkg_cycle = ?");
		q.addBindValue(udi);
		q.addBindValue(cycle);
		if (!q.exec())
			return kErrorDbUnreachable;
		if (!q.first())
			return "无此包配包记录";

		pack.op = q.value(0).toString();
		pack.time = q.value(1).toDateTime();
		pack.checker = q.value(2).toString();

		return 0;
	}

	result_t getSterilizeInfo(const QString & udi, int cycle,
		PackageFlow::Sterilize &ster, PackageFlow::SterilizeCheck &sterCheck)
	{
		QSqlQuery q;
		q.prepare("SELECT batch_id, device_name, program_name, cycle_count, cycle_total, start_time, finish_time, op_name,"
			" phy_check_result, phy_check_time, phy_check_op_name,"
			" che_check_result, che_check_time, che_check_op_name,"
			" bio_check_result, bio_check_time, bio_check_op_name"
			" FROM r_ster_batch"
			" WHERE batch_id = (SELECT batch_id FROM r_ster_package WHERE pkg_udi = ? AND pkg_cycle = ?)");
		q.addBindValue(udi);
		q.addBindValue(cycle);
		if (!q.exec())
			return kErrorDbUnreachable;
		if (!q.first())
			return "无此包灭菌记录";

		ster.batchId = q.value(0).toString();
		ster.device = q.value(1).toString();
		ster.program = q.value(2).toString();
		ster.cycleCount = q.value(3).toInt();
		ster.cycleTotal = q.value(4).toInt();
		ster.startTime = q.value(5).toDateTime();
		ster.finishTime = q.value(6).toDateTime();
		ster.op = q.value(7).toString();

		sterCheck.phyResult = static_cast<Rt::SterilizeVerdict>(q.value(8).toInt());
		sterCheck.phyCheckTime = q.value(9).toDateTime();
		sterCheck.phyChecker = q.value(10).toString();
		sterCheck.cheResult = static_cast<Rt::SterilizeVerdict>(q.value(11).toInt());
		sterCheck.cheCheckTime = q.value(12).toDateTime();
		sterCheck.cheChecker = q.value(13).toString();
		sterCheck.bioResult = static_cast<Rt::SterilizeVerdict>(q.value(14).toInt());
		sterCheck.bioCheckTime = q.value(15).toDateTime();
		sterCheck.bioChecker = q.value(16).toString();

		return 0;
	}

	result_t getDispatchInfo(const QString & udi, int cycle, PackageFlow::Dispatch &dispatch)
	{
		QSqlQuery q;
		q.prepare("SELECT op_name, dispatch_time, to_dept_name"
			" FROM r_dispatch"
			" WHERE pkg_udi = ? AND pkg_cycle = ?");
		q.addBindValue(udi);
		q.addBindValue(cycle);
		if (!q.exec())
			return kErrorDbUnreachable;
		if (!q.first())
			return "无此包发放记录";

		dispatch.op = q.value(0).toString();
		dispatch.time = q.value(1).toDateTime();
		dispatch.dept = q.value(2).toString();

		return 0;
	}

	result_t getSurgeryInfo(const QString & udi, int cycle,
		PackageFlow::SurgerBind &bind, PackageFlow::Use &use,
		PackageFlow::SurgeryPreCheck &preCheck, PackageFlow::SurgeryPostCheck &postCheck)
	{
		QSqlQuery q;
		q.prepare("SELECT id, surgery_name, surgery_time, patient_id, patient_name,"
			" bind_time, bind_op_name, pre_check_time, pre_check_op_name, post_check_time, post_check_op_name"
			" FROM r_surgery"
			" WHERE id = (SELECT surgery_id FROM r_surgery_package WHERE pkg_udi = ? AND pkg_cycle = ?)");
		q.addBindValue(udi);
		q.addBindValue(cycle);
		if (!q.exec())
			return kErrorDbUnreachable;
		if (!q.first())
			return "无此包清洗记录";

		use.surgeryId = q.value(0).toInt();
		use.surgeryName = q.value(1).toString();
		use.surgeryTime = q.value(2).toDateTime();
		use.patientId = q.value(3).toString();
		use.patientName = q.value(4).toString();

		bind.time = q.value(5).toDateTime();
		bind.op = q.value(6).toString();
		preCheck.time = q.value(7).toDateTime();
		preCheck.op = q.value(8).toString();
		postCheck.time = q.value(9).toDateTime();
		postCheck.op = q.value(10).toString();
		
		return 0;
	}

} // namespace

result_t TraceDao::getPackageFlow(const QString & udi, int cycle, PackageFlow *pf)
{
	if (!pf) return 0;

	// get flow status
	QSqlQuery q;
	q.prepare("SELECT status, recalled FROM r_package WHERE pkg_udi=? AND pkg_cycle=?");
	q.addBindValue(udi);
	q.addBindValue(cycle);
	if (!q.exec())
		return kErrorDbUnreachable;
	if (!q.first())
		return "无此包记录";
	pf->status = static_cast<Rt::FlowStatus>(q.value(0).toInt());
	pf->isRecalled = q.value(1).toBool();

	// get wash
	result_t res = Internal::getWashInfo(udi, cycle, pf->wash);
	if (!res.isOk()) return res;
	if (pf->status < Rt::Packed) return 0;

	// get pack
	res = Internal::getPackInfo(udi, cycle, pf->pack);
	if (!res.isOk()) return res;
	if (pf->status < Rt::Sterilized) return 0;

	// get sterilize and check
	res = Internal::getSterilizeInfo(udi, cycle, pf->ster, pf->sterCheck);
	if (!res.isOk()) return res;
	if (pf->status < Rt::Dispatched) return 0;

	// get dispatch
	res = Internal::getDispatchInfo(udi, cycle, pf->dispatch);
	if (!res.isOk()) return res;
	if (pf->status < Rt::SurgeryBound) return 0;

	// get surgery
	res = Internal::getSurgeryInfo(udi, cycle, pf->bind, pf->use, pf->preCheck, pf->postCheck);
	if (!res.isOk()) return res;

	return 0;
}

result_t TraceDao::getPatientSurgeries(int patientId, QList<Surgery> *surgeries)
{
	if (!surgeries) return 0;

	// get surgery ids
	QSqlQuery q;
	q.prepare("SELECT id FROM r_surgery WHERE patient_id=?");
	q.addBindValue(patientId);
	if (!q.exec())
		return kErrorDbUnreachable;

	QList<int> surgeryIds;
	while (q.next()) {
		surgeryIds << q.value(0).toInt();
	}
	if (0 == surgeryIds.size()) {
		return "未找到该病人的手术记录";
	}

	// get packages for each surgery
	SurgeryDao dao;
	Surgery sur;
	for each (int id in surgeryIds) {
		result_t res = dao.getSurgery(id, &sur, false);
		if (!res.isOk()) return res;
		surgeries->append(sur);
	}

	return 0;
}

