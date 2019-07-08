#include "datawrapper.h"
#include "core/itracnamespace.h"

namespace Sterile {

Result::Result() {
	physics = itrac::NotChecked;
	chemistry = itrac::NotInvolved;
	bio = itrac::NotInvolved;
}

QString Result::toString(int verdict) {
	switch (verdict)
	{
	case itrac::Success: return "1";
	case itrac::Failed: return "0";
	default: return QString();
	}
}

TestInfo::TestInfo() : cycle(-1) {};

}
