#pragma once

#include "enums.h"
#include "instrument.h"
#include "department.h"
#include <QList>
#include <QDate>

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
	Rt::SterilizeMethod sterMethod;
	PackType packType;
	Department dept;
	QList<DetailItem> detail;

	PackageType() :
		typeId(-1),
		category(Rt::UnknownPackage),
		sterMethod(Rt::UnknownSterilizeType)
	{}
};

struct Package : public PackageType
{
	QString udi;
	int cycle;
	Rt::FlowStatus status;
	QList<Instrument> instruments;
	
	Package() : cycle(0), status(Rt::UnknownFlowStatus) {}
};

struct PackageQualityControl {
	bool isExpired;
	bool isWetPack;
	bool isRecalled;
	Rt::SterilizeVerdict phyResult;
	Rt::SterilizeVerdict cheResult;
	Rt::SterilizeVerdict bioResult;
	QDate expireDate;
	
	PackageQualityControl() :
		isExpired(false), isWetPack(false), isRecalled(false),
		phyResult(Rt::Unchecked), cheResult(Rt::Unchecked), bioResult(Rt::Unchecked)
	{}
	
	PackageQualityControl(const PackageQualityControl& other) = default;
	PackageQualityControl& operator=(const PackageQualityControl& other) = default;
};

struct LabelInfo {
	QString labelId;
	QDate packDate;
	QDate expireDate;
};
