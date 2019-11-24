#include "application.h"
#include "launcher.h"
#include "apploader.h"
#include "mainwindow.h"
#include "user.h"

//#include "ui/composite/waitingspinner.h"

#include <qscanner/qscanner.h>
#include <QFile>

namespace Core {

Application::Application(Core::Launcher* launcher, int &argc, char **argv)
	: QApplication(argc, argv)
	, _launcher(launcher)
	, _op(new User)
{
	//setStyleSheet("QWidget{background-color:red;}"
	//"Picture{background-color:white;}");
	updateStyle();
	setupScanner();

	_loader = std::make_unique<AppLoader>();

	// showMaximized only affects windows(Qt::Window), and the widget's sizeHint is not (0, 0),
	// when show/showMaximized called, all child widgets are visible
	_loader->showFullScreen();
}

Application::~Application()
{
}

void Application::updateStyle()
{
	QFile file("style/default.style");
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QByteArray content = file.readAll();
		setStyleSheet(content);
		file.close();
	}
}

//void Application::startWaitingOn(QWidget* w) {
//	if (_waiter) { _waiter->stop(); }
//	_waiter.reset(new WaitingSpinner(w));
//	_waiter->start();
//}
//
//void Application::stopWaiting() {
//	if (_waiter) {
//		//_waiter->hide();
//		_waiter->stop();
//		_waiter.reset(nullptr);
//	}
//}

const Ui::Loader * Application::loader() const
{
	return _loader.get();
}

MainWindow * Application::mainWindow() const {
	return qobject_cast<MainWindow *>(_loader->source());
}

User & Application::currentUser() const {
	return *_op;
}

void Application::setupScanner()
{
	// HID-POS mode
	//IBarcodeScanner *scanner = BarcodeScannerFactory::Create(BarcodeScannerType::USB_HID_POS_NLOY20, NULL);

	// HID-KBW mode, need installation of event filter on QApplication/widgets
	IBarcodeScanner *scanner = BarcodeScannerFactory::Create(BarcodeScannerType::HID_KBW, NULL);
	scanner->start();
}

Application *app() {
	return static_cast<Application*>(qApp);
}

}
