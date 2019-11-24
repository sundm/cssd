#include "core/launcher.h"

#define ERR_LAUNCH 1

int main(int argc, char *argv[]) {
	const auto launcher = Core::Launcher::create(argc, argv);
	return launcher ? launcher->start(): ERR_LAUNCH;
}