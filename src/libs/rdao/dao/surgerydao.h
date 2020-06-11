# pragma once

#include "../rdao_global.h"
#include "../entity/surgery.h"
#include <rcore/result.h>

class RDAO_EXPORT SurgeryDao
{
public:

	result_t getSurgeryList(Rt::SurgeryStatus status, QList<Surgery> *surgeries);

	result_t getSurgery(int surgeryId, Surgery *surgery, bool excludeBasicInfo = true);

	result_t addSurgery(const Surgery &surgery);

	result_t delSurgery(int surgeryId);
};

