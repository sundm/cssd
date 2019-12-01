#pragma once 

#include "enums.h"
#include <QString>
#include <QDateTime>

struct Program {
	int id;
	Rt::DeviceCategory category;
	QString name;
	QString remark;

	Program() : category(Rt::UnknownDeviceCategory) {}
};

struct Device{
	int id;
	QString name;
	Rt::DeviceCategory category;
	Rt::DeviceStatus status;
	int cycleToday;
	int cycleTotal;
	Rt::SterilizeType sterilizeType;
	QDate productionDate;
	QDateTime lastMaintainTime;
	unsigned int maintainCycle;
	QList<Program> programs;

	Device() :
		id(-1),
		category(Rt::UnknownDeviceCategory),
		status(Rt::Idle),
		cycleToday(-1),
		cycleTotal(-1),
		sterilizeType(Rt::UnknownSterilizeType),
		maintainCycle(0)
	{}
};

typedef Device Washer;
typedef Device Sterilizer;

struct DeviceBatchInfo {
	QString batchId;
	QString deviceName;
	QString programName;

};
