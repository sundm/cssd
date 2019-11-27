#pragma once

#include "enums.h"
#include <QString>

struct InstrumentType
{
	int typeId;
	Rt::InstrumentCategory category;
	QString name;
	QString pinyin;
	QString photo;
	bool isVip;

	InstrumentType() :
		typeId(-1),
		category(Rt::NormalInstrument),
		isVip(false)
	{ }
};

struct Instrument : public InstrumentType
{
	QString udi;
	QString packageUdi;
	int price;

	Instrument() : price(-1) {}
};
