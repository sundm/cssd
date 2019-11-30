# pragma once

#include "../rdao_global.h"
#include <rcore/result.h>

struct Operator;
struct Package;
struct Device;
struct Program;

class RDAO_EXPORT FlowDao
{
public:

	result_t addRecycle(const Package &pkg, const Operator& op);

	result_t addWash(
		int deviceId,
		const Program &program,
		const QList<Package> &pkgs,
		const Operator &op);
	
private:
};

