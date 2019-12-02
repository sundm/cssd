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

struct SterilizeResult {
	Rt::SterilizeVerdict phyVerdict;
	Rt::SterilizeVerdict cheVerdict;
	Rt::SterilizeVerdict bioVerdict;
	bool hasWetPack;
	bool hasLabelOff;

	SterilizeResult() :
		phyVerdict(Rt::Unchecked),
		cheVerdict(Rt::Unchecked),
		bioVerdict(Rt::Unchecked),
		hasWetPack(false),
		hasLabelOff(false)
	{}

	bool isPhyVerdictValid() const { return Rt::Qualified == phyVerdict || Rt::Unqualified == phyVerdict; }
	bool isCheVerdictValid() const { return Rt::Qualified == cheVerdict || Rt::Unqualified == cheVerdict; }
	bool isBioVerdictValid() const { return Rt::Unchecked < bioVerdict && Rt::Uninvolved >= bioVerdict; }
};

struct DeviceBatchInfo {
	struct BatchPackageItem {
		QString udi;
		QString name;
		int cycle;
		bool isWetPack;
	};

	QString batchId;
	QString deviceName;
	QString programName;
	QString opName;
	unsigned int cycleCount;
	unsigned int cycleTotal;
	QDateTime startTime;
	QDateTime finishTime;
	SterilizeResult result;
	QList<BatchPackageItem> packages;
};
