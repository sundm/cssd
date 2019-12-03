#pragma once

#include <QDateTime>
#include "enums.h"
#include "package.h"

struct Surgery {

	struct DetailItem {
		int pkgTypeId;
		int pkgNum;
		QString pkgTypeName;

		DetailItem() : pkgTypeId(0), pkgNum(0) {}
		DetailItem(int pkgTypeId, int pkgNum, QString &pkgTypeName = QString()) :
			pkgTypeId(pkgTypeId),
			pkgNum(pkgNum),
			pkgTypeName(pkgTypeName)
		{}
	};

	int id;
	QString surgeryName;
	QDateTime surgeryTime;
	QString surgeryRoom;
	int patientId;
	QString patientName;
	int patientAge;
	Rt::Gender patientGender;

	QList<DetailItem> detail;
	QList<Package> packages;

	Surgery() :
		id(0),
		patientId(0),
		patientAge(0),
		patientGender(Rt::UnknownGender)
	{}
};
