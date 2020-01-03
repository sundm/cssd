#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "errors.h"
#include "recalldao.h"
#include "../entity/recall.h"

result_t RecallDao::getSterilizeBatchInfo(const QString &udi, SterBatchInfo *sbi)
{
	return 0;
}

result_t RecallDao::getSterilizeBatchInfo(int deviceId, int cycleTotal, SterBatchInfo *sbi)
{
	return 0;
}

result_t RecallDao::addRecall(int deviceId, int cycleTotal, const Operator &op, const QString desc /*= QString()*/)
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
