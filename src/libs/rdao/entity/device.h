#pragma once 

#include "enums.h"
#include <QString>
#include <QDateTime>
#include "package.h"

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
	Rt::SterilizeMethod sterilizeMethod;
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
		sterilizeMethod(Rt::UnknownSterilizeType),
		maintainCycle(0)
	{}
};

typedef Device Washer;
typedef Device Sterilizer;

struct SterilizeResult {

	// struct PackageItem indicates whether a package is wetpack or not
	struct PackageItem {
		QString udi;
		int cycle;
		bool isWetPack;
		QString name;

		PackageItem() : cycle(-1), isWetPack(false) {}
		PackageItem(const QString &udi, int cycle, bool isWetPack=false, const QString &name=QString()) :
			udi(udi), cycle(cycle), isWetPack(isWetPack), name(name)
		{}
	};

	// members
	Rt::SterilizeVerdict phyVerdict;
	Rt::SterilizeVerdict cheVerdict;
	Rt::SterilizeVerdict bioVerdict;
	bool hasLabelOff;
	QList<PackageItem> packages;

	SterilizeResult() :
		phyVerdict(Rt::Unchecked),
		cheVerdict(Rt::Unchecked),
		bioVerdict(Rt::Unchecked),
		hasLabelOff(false)
	{}

	bool isPhyVerdictValid() const { return Rt::Qualified == phyVerdict || Rt::Unqualified == phyVerdict; }
	bool isCheVerdictValid() const { return Rt::Qualified == cheVerdict || Rt::Unqualified == cheVerdict; }
	bool isBioVerdictValid() const { return Rt::Unchecked < bioVerdict && Rt::Uninvolved >= bioVerdict; }

	static Rt::SterilizeVerdict determineVerdict(
		Rt::SterilizeVerdict phyVerdict,
		Rt::SterilizeVerdict cheVerdict,
		Rt::SterilizeVerdict bioVerdict) {
		if (Rt::Qualified == phyVerdict && Rt::Qualified == cheVerdict &&
			(Rt::Qualified == bioVerdict || Rt::Uninvolved == bioVerdict))
			return Rt::Qualified;
		return Rt::Unqualified;
	}
};

struct DeviceBatchInfo {
	QString batchId;
	QString deviceName;
	QString programName;
	QString opName;
	unsigned int cycleCount;
	unsigned int cycleTotal;
	QDateTime startTime;
	QDateTime finishTime;
	SterilizeResult result;
};

struct DynamicSterBatchInfo {
	QString batchId;
	QString programName;
	QDate date;
	unsigned int cycleCount;
	unsigned int cycleTotal;
};

struct SterBatchInfo : public DynamicSterBatchInfo {
	int deviceId;
	QString deviceName;
	QList<Package> packages;
};

struct RangedSterBatchInfo {
	int deviceId;
	QString deviceName;
	QList<DynamicSterBatchInfo> bis;
};
