#include "assets.h"
#include "itracnamespace.h"

#include "core/net/url.h"

namespace {
	inline int getPackageState(const QString &s) {
		if (s == "P") return itrac::Packed;
		if (s == "S") return itrac::Sterilized;
		if (s == "ST") return itrac::SteExamined;
		if (s == "I") return itrac::Dispatched;
		if (s == "U") return itrac::Used;
		if (s == "R") return itrac::Recycled;
		if (s == "X") return itrac::Recalled;
		return itrac::UnknownState;
	}
}

Device::Device()
	: cycleToday(-1)
	, cycleSum(-1)
	, type(-1)
	, state(Device::Disabled)
	, bdResult(itrac::NotInvolved)
{};

int Device::tranlateState(const QString &s) {
	if (s == "0") return Device::Idle;
	if (s == "2") return Device::Running;
	return Device::Disabled;
}

Package::Package()
	:deptId(-1)
	, state(itrac::Packed)
	, expired(false)
	, steQualified(false)
{}

