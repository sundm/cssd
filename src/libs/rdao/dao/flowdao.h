# pragma once

#include "../rdao_global.h"
#include <rcore/result.h>

class Operator;
class Package;
class RDAO_EXPORT FlowDao
{
public:

	result_t addRecycle(const Package &pkg, const Operator& op);

	result_t addWash(const QList<Package> &pkgs, const Operator& op);
	
};

