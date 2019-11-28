#pragma once 

#include "enums.h"
#include <QString>
#include <QDateTime>

struct Device{
	int id;
	QString name;
	Rt::DeviceCategory category;
	Rt::Status status;
	int cycleCount;
	int cycleTotal;
	QDate cycleDate;
	QDate productionDate;
	QDateTime lastMaintainTime;
	unsigned int maintainCycle;

	Device() :
		id(-1),
		category(Rt::UnknownDevice),
		status(Rt::Normal),
		cycleCount(-1),
		cycleTotal(-1),
		maintainCycle(0)
	{}
};

typedef Device Washer;

struct Sterilizer : public Device {
	Rt::SterilizeType sterilizeType;

	Sterilizer() :
		Device(),
		sterilizeType(Rt::BothTemperature)
	{}
};
