#include "datawrapper.h"
#include "core/itracnamespace.h"

namespace Sterile {

Result::Result() {
	physics = Rt::SterilizeVerdict::Unchecked;
	chemistry = Rt::SterilizeVerdict::Unchecked;
	bio = Rt::SterilizeVerdict::Unchecked;
}

TestInfo::TestInfo() : cycle(-1) {};

}
