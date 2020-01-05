#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "errors.h"
#include "recalldao.h"
#include "../entity/recall.h"
#include "../entity/device.h"

result_t RecallDao::getSterilizeBatchInfo(const QString &udi, SterBatchInfo *sbi)
{
	if (!sbi) return 0;

	// get the latest cycle
	QSqlQuery q;
	q.prepare("SELECT cycle FROM t_package WHERE udi = ?");
	q.addBindValue(udi);
	if (!q.exec()) return q.lastError().text();
	if (!q.first()) return "未找到此包信息，请确认该包已入库";
	int cycle = q.value(0).toInt();

	// get sterilize batch info
	q.prepare("SELECT batch_id, device_id, device_name, program_name, cycle_count, cycle_total, start_time"
		" FROM r_ster_batch"
		" WHERE batch_id = (SELECT batch_id FROM r_ster_package WHERE pkg_udi=? AND pkg_cycle=?)");
	q.addBindValue(udi);
	q.addBindValue(cycle);
	if (!q.exec()) return q.lastError().text();
	if (!q.first()) return "未找到该包对应的灭菌锅次信息";
	sbi->batchId = q.value(0).toString();
	sbi->deviceId = q.value(1).toInt();
	sbi->deviceName = q.value(2).toString();
	sbi->programName = q.value(3).toString();
	sbi->cycleCount = q.value(4).toInt();
	sbi->cycleTotal = q.value(5).toInt();
	sbi->date = q.value(6).toDate();

	// get sibling packages
	// TODO: we should integrate `r_ster_package` into `r_package`, or it's rather complicated to fetch siblings 

	return 0;
}

result_t RecallDao::getSterilizeBatchInfo(int deviceId, int cycleTotal, SterBatchInfo *sbi)
{
	if (!sbi) return 0;

	// TODO 
	QSqlQuery q;
	q.prepare("SELECT batch_id, device_name, program_name, cycle_count, start_time"
		" FROM r_ster_batch"
		" WHERE device_id = ? AND cycle_total = ?");
	q.addBindValue(deviceId);
	q.addBindValue(cycleTotal);
	if (!q.exec()) return q.lastError().text();
	if (!q.first()) return "未找到指定的灭菌锅次信息";
	sbi->batchId = q.value(0).toString();
	sbi->deviceId = deviceId;
	sbi->deviceName = q.value(1).toString();
	sbi->programName = q.value(2).toString();
	sbi->cycleCount = q.value(3).toInt();
	sbi->cycleTotal = cycleTotal;
	sbi->date = q.value(4).toDate();

	// get sibling packages
	// TODO: we should integrate `r_ster_package` into `r_package`, or it's rather complicated to fetch siblings 

	return 0;
}

result_t RecallDao::addRecall(int deviceId, int cycleTotal, const Operator &op, const QString &desc /*= QString()*/)
{
	return 0;
}

result_t RecallDao::addRecall(int deviceId, int startCycleTotal, int endCycleTotal, const Operator &op)
{
	return 0;
}

result_t RecallDao::getBatchesToBeRecalled(QList<RangedSterBatchInfo> *rangedSbi)
{
	return 0;
}
