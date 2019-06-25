#include "launcher.h"
#include "platform/platfom_specific.h"

#include "application.h"

namespace Core {

	Launcher::Launcher(int argc, char* argv[], const QString &sysVersion)
		:_argc(argc), _argv(argv), _sysVersion(sysVersion) {
	}

	std::unique_ptr<Launcher> Launcher::create(int argc, char *argv[])
	{
		return std::make_unique<Platform::Launcher>(argc, argv);
	}

	int Launcher::start()
	{
		initialize();

		Application app(this, _argc, _argv);
		return app.exec();
	}

	QString Launcher::sysVersion() const {
		return _sysVersion;
	}

	void Launcher::prepareSettings() {
	}

	void Launcher::initialize() {
		QCoreApplication::setApplicationName("Winstrac Client Powered By Winstech");
		prepareSettings();
	}

}
