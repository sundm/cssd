#pragma once

#include "enums.h"
#include "instrument.h"
#include "department.h"
#include <QList>

struct PackType {
	int id;
	QString name;
	unsigned int validPeriod;
	unsigned int standardPeriod;
	PackType() : id(-1), validPeriod(0),standardPeriod(0) {}
};

struct PackageType
{
	struct DetailItem {
		int insTypeId;
		int insNum;
		QString insName;

		DetailItem() = default;
		DetailItem(int typeId, int num, QString &name = QString()) :
			insTypeId(typeId),
			insNum(num),
			insName(name)
		{}
	};

	int typeId;
	Rt::PackageCategory category;
	QString name;
	QString pinyin;
	QString photo;
	Rt::SterilizeType sterType;
	PackType packType;
	Department dept;
	QList<DetailItem> detail;

	PackageType() : typeId(-1), category(Rt::UnknownPackage) {}
};

struct Package : public PackageType
{
	QString udi;
	QList<Instrument> instruments;
};
