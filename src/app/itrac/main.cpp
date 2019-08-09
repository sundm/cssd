#include "core/launcher.h"

#define ERR_LAUNCH 1

int main(int argc, char *argv[]) {

	const auto launcher = Core::Launcher::create(argc, argv);
	return launcher ? launcher->start(): ERR_LAUNCH;
	
	// setup fonts
	//a.setFont(QFont("Microsoft Yahei", 9));
	//int fontId = QFontDatabase::addApplicationFont(":/res/fontawesome-webfont.ttf");
	//QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
	//qDebug() << "fontFamilies.size() " << fontFamilies.size();

	// setup event filter for Scanning (HID_KBW mode)
	//ScanEater *se = new ScanEater(0);
	//qApp->installEventFilter(se); // or use a.installEventFilter(se);
}