#include "launcher.h"
#include "platform/platfom_specific.h"

#include "application.h"
#include "core/net/url.h"
#include <QSettings>

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
		//QFont font = app.font();
		//font.setPointSize(12);
		//app.setFont(font);
		return app.exec();
	}

	QString Launcher::sysVersion() const {
		return _sysVersion;
	}

	void Launcher::prepareSettings() {
		QSettings *configIni = new QSettings("prepareSettings.ini", QSettings::IniFormat);

		PATH_BASE = configIni->value("address/t").toString();

		LABEL_PRINTER = configIni->value("printer/label").toString();

		COMMON_PRINTER = configIni->value("printer/common").toString();

		delete configIni;
	}

	void Launcher::initialize() {
		QCoreApplication::setApplicationName("Winstrac Client Powered By Winstech");
		prepareSettings();
	}

}
