#pragma once

#include "enums.h"
#include <QString>

struct InstrumentType
{
	int typeId;
	QString name;
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
	int cycle;
	QString packageUdi;
	int price;

	Instrument() : cycle(0), price(-1) {}
};
