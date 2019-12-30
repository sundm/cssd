#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "errors.h"
#include "recalldao.h"
#include "../entity/recall.h"

namespace Internal {

} // namespace

result_t RecallDao::getRecallPackagesByDevice(int deviceId, int cycle, QList<PackageForRecall> *packages)
{
	return 0;
}

result_t RecallDao::getRecallByPackageId(const QString &udi, RecallItem *recall)
{
	return 0;
}

result_t RecallDao::getCurrentRecallList(QList<RecallItem> *recalls)
{
	return 0;
}


result_t RecallDao::getHistoryRecallList(const Filtrate &filtrate, QList<RecallItem> *recalls)
{
	return 0;
}


result_t RecallDao::addBatchRecalls(const QList<RecallItem> &recalls)
{
	return 0;
}


result_t RecallDao::addSingleRecall(const RecallItem &recall)
{
	return 0;
}
