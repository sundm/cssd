# pragma once

#include "../rdao_global.h"
#include "../entity/department.h"
#include <rcore/result.h>

class RDAO_EXPORT DeptDao
{
public:
	result_t getDept(int id, Department *dept);

	result_t getDeptList(QList<Department> *depts);
};

