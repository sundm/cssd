# pragma once

#include "../rdao_global.h"
#include "../entity/enums.h"
#include <rcore/result.h>

struct Operator;
struct Package;
struct Device;
struct Program;
struct LabelInfo;

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
	
private:
	result_t updatePackageStatus(const Package &pkg, Rt::FlowStatus fs);
};

