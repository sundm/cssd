#include "launcher_win.h"

//#include <windows.h>
//#include <shellapi.h>
#include <QSysInfo>

namespace Platform {

namespace {

	QString SystemVersion() {
		switch (QSysInfo::windowsVersion()) {
		case QSysInfo::WV_XP: return "Windows XP";
		case QSysInfo::WV_2003: return "Windows 2003";
		case QSysInfo::WV_VISTA: return "Windows Vista";
		case QSysInfo::WV_WINDOWS7: return "Windows 7";
		case QSysInfo::WV_WINDOWS8: return "Windows 8";
		case QSysInfo::WV_WINDOWS8_1: return "Windows 8.1";
		case QSysInfo::WV_WINDOWS10: return "Windows 10";
		default: return "Windows";
		}
	}

} // namespace

Launcher::Launcher(int argc, char *argv[])
	: Core::Launcher(argc, argv, SystemVersion()) {
}

//bool Launcher::launch(
//	const QString &operation,
//	const QString &binaryPath,
//	const QStringList &argumentsList) {
//	return true;
//}

} // namespace Platform
