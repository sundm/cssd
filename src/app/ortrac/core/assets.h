#pragma once

#include <QString>
#include <functional>

struct Asset {
	int id;
	QString name;
	QString err;
};

struct Plate : public Asset {
	bool idle;
};

struct Device : public Asset {
	enum State { Idle, Running, Disabled };
	enum Type { Washer, Sterilizer };
	int cycleToday;
	int cycleSum;
	int type;
	int state;
	int bdResult;

	Device();
	bool isIdle() const { return state == Device::Idle; }
	QString typeValue() const {
		switch (type) {
		case Washer: return washerType;
		case Sterilizer: return sterilizerType;
		default: return QString();
		}
	}
	void setTypeValue(const QString &value) {
		if (washerType == value) type = Washer;
		else if (sterilizerType == value) type = Sterilizer;
		else type = -1;
	}
	static int tranlateState(const QString &);

	static constexpr char washerType[] = "0001";
	static constexpr char sterilizerType[] = "0002";
};

struct Package : public Asset{
	QString label;
	QString expireDate;
	QString packName;
	QString usedBy;
	QString deparment;
	int deptId;
	int state;
	bool expired;
	bool steQualified;

	Package();
};

