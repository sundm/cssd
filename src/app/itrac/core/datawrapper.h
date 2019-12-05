#pragma once

#include <QString>
#include "rdao/entity/enums.h"

namespace Sterile {

struct Result {
	Result();

	Rt::SterilizeVerdict physics;
	Rt::SterilizeVerdict chemistry;
	Rt::SterilizeVerdict bio;
	Rt::SterilizeVerdict lost;
};


struct TestInfo {
	QString testId;
	QString device;
	int cycle;
	QString startTimeStamp;
	//Result result;

	TestInfo();
};

} // namespace Sterile
