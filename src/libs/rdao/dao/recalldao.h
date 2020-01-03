# pragma once

#include "../rdao_global.h"
#include <rcore/result.h>

struct Operator;
struct SterBatchInfo;
struct RangedSterBatchInfo;

class RDAO_EXPORT RecallDao
{
public:

	/**
	 * Get the corresponding sterilize batch information by a specified package.
	 * When index the batch number, the package's latest cycle is used. The returned sibling
	 * packages contains udi, name, and status.
	 * @param udi: the package udi
	 * @param sbi[OUT]: the object used to receive information
	 */
	result_t getSterilizeBatchInfo(const QString &udi, SterBatchInfo *sbi);

	/**
	 * This function is an overload version.
	 * Get the corresponding sterilize batch information by the device id and its cycle.
	 * The returned sibling packages contains udi, name, and status.
	 * @param deviceId: the device id
	 * @param cycleTotal: the device total cycle
	 * @param sbi[OUT]: the object used to receive information
	 */
	result_t getSterilizeBatchInfo(int deviceId, int cycleTotal, SterBatchInfo *sbi);

	/**
	 * Recall a single batch of packages, the batch is indexed by id & total cycle of the device.
	 * @param deviceId: the package udi
	 * @param cycleTotal: the device total cycle
	 * @param op: the operator
	 * @param desc: the description, usually elaborates the reason of the recall 
	 */
	result_t addRecall(
		int deviceId,
		int cycleTotal,
		const Operator &op,
		const QString &desc = QString());

	/**
	 * This function is an overload version.
	 * Recall several batches which are specified by the device id and a range of total cycles.
	 * @param deviceId: the package udi
	 * @param startCycleTotal: the start total cycle
	 * @param endCycleTotal: the end total cycle
	 * @param op: the operator
	 */
	result_t addRecall(
		int deviceId,
		int startCycleTotal,
		int endCycleTotal, 
		const Operator &op);

	result_t getBatchesToBeRecalled(QList<RangedSterBatchInfo> *rangedSbi);

	//result_t getHistoryRecallList(const Filtrate &filtrate, QList<RecallItem> *recalls);
};

