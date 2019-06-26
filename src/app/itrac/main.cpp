#include "core/launcher.h"

#include <printer/labelprinter.h>
#include "util/printermanager.h"

#define ERR_LAUNCH 1

const QString version("V1.0.0");

void testPrinter()
{
	LabelPrinter *printer = PrinterManager::currentPrinter();
	if (0 != printer->open("ZDesigner GT800 (ZPL)")) {
		return;
	}
	printer->setInversion(true);
	PackageLabel label;
	label.packageId = "100000861904230005";
	label.packageName = "胸腔器械（每份84把共1份）";
	label.packageFrom = "手术室";
	label.packageType = "棉布";
	label.disinDate = "2019-04-23 18:47:02";
	label.expiryDate = "2019-04-30";
	label.operatorName = "张爱琴";
	label.assessorName = "李建";
	label.count = 54;

	printer->printPackageLabel(label);

	printer->close();
}

int main(int argc, char *argv[]) {
	/*testPrinter();
	return 0;*/

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