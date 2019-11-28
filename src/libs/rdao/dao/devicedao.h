# pragma once

#include "../rdao_global.h"
#include "../entity/device.h"
#include <rcore/result.h>

class RDAO_EXPORT DeviceDao
{
public:
	result_t getDevice(int id, Device *device, bool withPrograms = false);

	/**
	 * Fetch washer devices.
	 * @param washers: out param
	 * @param onlyAvailable: if true, those frozen are not returned, otherwise return all
	 * @return a result_t object
	 */
	result_t getWasherList(
		QList<Washer> *washers,
		bool onlyAvailable = true
	);

	result_t getSterilizerList(
		QList<Sterilizer> *sterilizers,
		bool onlyAvailable = true
	);

	result_t getAllDeivces(
		QList<Device> *devices,
		bool onlyAvailable = true
	);

	result_t getProgramsForDevice(int deviceId, QList<Program> *programs);

	result_t getWashPrograms(QList<Program> *programs);
	result_t getSterilizePrograms(QList<Program> *programs);
	result_t getAllPrograms(QList<Program> *programs);

private:
	result_t getDeviceList(
		Rt::DeviceCategory cat,
		QList<Device> *devices,
		bool onlyAvailable = true
	);

	result_t getProgramList(
		Rt::DeviceCategory cat,
		QList<Program> *programs
	);
};

