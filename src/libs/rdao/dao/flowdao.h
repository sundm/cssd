# pragma once

#include "../rdao_global.h"
#include "../entity/enums.h"
#include <rcore/result.h>

struct Operator;
struct Package;
struct Device;
struct Program;
struct Department;
struct LabelInfo;
struct DeviceBatchInfo;
struct SterilizeResult;
struct Surgery;

class RDAO_EXPORT FlowDao
{
public:

	result_t addRecycle(const Package &pkg, const Operator& op);

	result_t addWash(
		int deviceId,
		const Program &program,
		const QList<Package> &pkgs,
		const Operator &op);

	result_t addPack(
		const Package &pkg,
		const Operator &op,
		const Operator &checker,
		LabelInfo *li);

	result_t addSterilization(
		int deviceId,
		const Program &program,
		const QList<Package> &pkgs,
		const Operator &op);

	/**
	 * Chemical verdict, physical result, label-off and wet-pack should be
	 * always committed together, while bio result can be committed separately.
	 */
	result_t updateSterilizationResult(
		const QString &batchId,
		const Operator &op,
		const SterilizeResult &result);

	result_t getDeviceBatchInfoByPackage(
		const Package &pkg,
		DeviceBatchInfo *dbi);

	/**
	 * When add a dispatch, we do not check if the package is valid(expired, wet-pack, recalled,
	 * sterilize-unqualified, etc.), the App should do these checks.
	 */
	result_t addDispatch(
		const QList<Package> &pkgs,
		const Department &dept,
		const Operator &op);

	result_t addSurgeryBindPackages(
		const Surgery &surgery,
		const Operator &op);

	result_t addSurgeryPreCheck(
		int surgeryId,
		const Operator &op);
	
	result_t addSurgeryPostCheck(
		int surgeryId,
		const Operator &op);
	
	void testRollback();

private:
	result_t updatePackageStatus(const Package &pkg, Rt::FlowStatus fs);

	result_t updatePackageStatus(const QList<Package> &pkgs, Rt::FlowStatus fs);

	result_t addDeviceBatch(
		int deviceId,
		const Program &program,
		const QList<Package> &pkgs,
		const Operator &op);

	result_t getPackagesInBatch(const QString &batchId, QList<Package> *pkgs);
	result_t getPackagesInSurgery(int surgeryId, QList<Package> *pkgs);
	result_t addSurgeryCheck(int surgeryId, const Operator &op, bool pre/* = true*/);
};

