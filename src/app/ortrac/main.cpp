#include "ortrac.h"
#include "core/launcher.h"

int main(int argc, char *argv[])
{
	const auto launcher = Core::Launcher::create(argc, argv);
	return launcher ? launcher->start() : -1;
}
