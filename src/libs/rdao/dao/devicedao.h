# pragma once

#include "../rdao_global.h"
#include "../entity/device.h"
#include <rcore/result.h>

class RDAO_EXPORT DeviceDao
{
public:

	result_t getWasherList(
		QList<Washer> *washers,
		int page = 1,
		int count = -1);

	result_t getSterilizerList(
		QList<Sterilizer> *sterilizers,
		int page = 1,
		int count = -1);

private:
	result_t getDeviceList(Rt::DeviceCategory cat, Device *);
};

