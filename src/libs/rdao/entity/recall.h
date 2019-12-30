#pragma once

#include "enums.h"
#include <QDateTime>

struct Filtrate {
	QDateTime fromTime;
	QDateTime toTime;

	int opId;
	int deviceId;

	Rt::RecallReason reason;
};

struct PackageForRecall {
	QString udi;
	QString name;

	struct _use_ {
		bool isUsed;
		QString patientId;
		QString patientName;
	};
};

struct RecallItem {
	int deviceId;
	QString deviceName;
	int cycle;

	struct _optime_ {
		int opId;
		QString opName;
		QDateTime time;
	};

	QList<PackageForRecall> packages;

	Rt::RecallReason reason;
	QString describe;
};
