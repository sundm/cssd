#pragma once

#include "core/launcher.h"

namespace Platform {

class Launcher : public Core::Launcher {
public:
	Launcher(int argc, char *argv[]);

private:

	//bool launch(
	//	const QString &operation,
	//	const QString &binaryPath,
	//	const QStringList &argumentsList);

};

} // namespace Platform
