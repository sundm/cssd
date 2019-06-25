#pragma once

#include <QString>

namespace Sterile {

struct Result {
	Result();

	int physics;
	int chemistry;
	int bio;
	static QString toString(int);
};


struct TestInfo {
	QString testId;
	QString device;
	int cycle;
	QString startTimeStamp;
	Result result;

	TestInfo();
};

} // namespace Sterile
