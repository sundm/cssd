# pragma once

#include "../rdao_global.h"
#include <rcore/result.h>

class RecallItem;
class Filtrate;
class PackageForRecall;

class RDAO_EXPORT RecallDao
{
public:
	result_t getRecallPackagesByDevice(int deviceId, int cycle, QList<PackageForRecall> *packages);

	result_t getRecallByPackageId(const QString &udi, RecallItem *recall);

	result_t getCurrentRecallList(QList<RecallItem> *recalls);

	result_t getHistoryRecallList(const Filtrate &filtrate, QList<RecallItem> *recalls);

	result_t addBatchRecalls(const QList<RecallItem> &recalls);

	result_t addSingleRecall(const RecallItem &recall);
};

