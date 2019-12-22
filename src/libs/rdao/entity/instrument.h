#pragma once

#include "enums.h"
#include <QString>

struct InstrumentType
{
	int typeId;
	QString typeName;
	QString pinyin;
	QString photo;
	bool isVip;

	InstrumentType() :
		typeId(-1),
		isVip(false)
	{ }
};

struct Instrument : public InstrumentType
{
	QString udi;
	QString name;
	QString alias;
	QString packageUdi;
	QString packageName;
	int packageCycle;
	int cycle;
	int price;

	Instrument() : packageCycle(0), cycle(0), price(-1) {}
};
