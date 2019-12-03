# pragma once

#include "../rdao_global.h"
#include "../entity/surgery.h"
#include <rcore/result.h>

class RDAO_EXPORT SurgeryDao
{
public:

	result_t getSurgeryList(Rt::SurgeryStatus status, QList<Surgery> *surgeries);

	result_t addSurgery(const Surgery &surgery);

};

