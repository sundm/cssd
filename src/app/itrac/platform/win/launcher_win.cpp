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
//	const auto convertPath = [](const QString &path) {
//		return QDir::toNativeSeparators(path).toStdWString();
//	};
//	const auto nativeBinaryPath = convertPath(binaryPath);
//	const auto nativeWorkingDir = convertPath(cWorkingDir());
//	const auto arguments = argumentsList.join(' ');
//
//	DEBUG_LOG(("Application Info: executing %1 %2"
//		).arg(binaryPath
//		).arg(arguments
//		));
//
//	const auto hwnd = HWND(0);
//	const auto result = ShellExecute(
//		hwnd,
//		operation.isEmpty() ? nullptr : operation.toStdWString().c_str(),
//		nativeBinaryPath.c_str(),
//		arguments.toStdWString().c_str(),
//		nativeWorkingDir.empty() ? nullptr : nativeWorkingDir.c_str(),
//		SW_SHOWNORMAL);
//	if (long(result) < 32) {
//		DEBUG_LOG(("Application Error: failed to execute %1, working directory: '%2', result: %3"
//			).arg(binaryPath
//			).arg(cWorkingDir()
//			).arg(long(result)
//			));
//		return false;
//	}
//	return true;
//}

} // namespace Platform
