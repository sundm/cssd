#pragma once

#include "enums.h"
#include <QDateTime>

struct PackageFlow {
	struct _optime_ {
		QString op;
		QDateTime time;
	};

	struct _device_{
		QString batchId;
		QString device;
		QString program;
		QString op;
		int cycleCount;
		int cycleTotal;
		QDateTime startTime;
		QDateTime finishTime;
		_device_() :cycleCount(0), cycleTotal(0) {}
	};
	
	typedef struct _device_ Wash, Sterilize;

	struct Pack : public _optime_{
		QString checker;
	};

	struct SterilizeCheck {
		QString phyChecker;
		QString cheChecker;
		QString bioChecker;
		QDateTime phyCheckTime;
		QDateTime cheCheckTime;
		QDateTime bioCheckTime;
		Rt::SterilizeVerdict phyResult;
		Rt::SterilizeVerdict cheResult;
		Rt::SterilizeVerdict bioResult;
		SterilizeCheck() : phyResult(Rt::Unchecked), cheResult(Rt::Unchecked), bioResult(Rt::Unchecked) {}
	};

	struct Dispatch : public _optime_{
		QString dept;
	};

	typedef struct _optime_ SurgerBind, SurgeryPreCheck, SurgeryPostCheck;

	struct Use {
		QString patientId;
		QString patientName;
		QString surgeryName;
		QDateTime surgeryTime;
		// TODO: surgeryId?
	};

	Rt::FlowStatus status;
	Wash wash;
	Pack pack;
	Sterilize ster;
	SterilizeCheck sterCheck;
	Dispatch dispatch;
	SurgerBind bind;
	Use use;
	SurgeryPreCheck preCheck;
	SurgeryPostCheck postCheck;
};
