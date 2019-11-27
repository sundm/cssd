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
	Rt::SterilizeType sterilizeType;
	QDate productionDate;
	QDateTime lastMaintainTime;
	unsigned int maintainCycle;

	Device() :
		id(-1),
		category(Rt::UnknownDevice),
		status(Rt::Normal),
		cycleCount(-1),
		cycleTotal(-1),
		sterilizeType(Rt::BothTemperature),
		maintainCycle(0)
	{}
};

struct PackageType
{
	typedef struct {
		int insTypeId;
		QString insName;
		int insNum;
	} DetailItem;

	enum Category {
		Surgical,
		Clinical,
		External,
		Dressing,
		Universal,
		Unknown
	};
	
	int typeId;
	Category category;
	QString name;
	QString pinyin;
	QString photo;
	SterilizeType sterType;
	PackType packType;
	QList<DetailItem> detail;

	PackageType() : category(Unknown) {}
};

struct Package : public PackageType
{
	QString udi;
	QList<Instrument> instruments;
};

#endif // PACKAGE_H
