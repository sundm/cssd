#pragma once

#include "enums.h"
#include "instrument.h"
#include "department.h"
#include <QList>

struct PackType {
	int id;
	QString name;
	unsigned int validPeriod;
	PackType() : id(-1), validPeriod(0) {}
};

struct PackageType
{
	typedef struct {
		int insTypeId;
		QString insName;
		int insNum;
	} DetailItem;

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
