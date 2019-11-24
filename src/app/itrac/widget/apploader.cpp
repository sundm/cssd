#include "apploader.h"
#include "widget/loginpage.h"
#include "widget/mainwindow.h"
#include "ui/ui_commons.h"

AppLoader::AppLoader(QWidget *parent)
	: Ui::Loader(parent)
{
	/*resize(1920, 540);*/
	// combine a fullscreen shortcut
	Ui::addPrimaryShortcut(this, Qt::Key_F11, SLOT(toggleFullScreen()));

	LoginPage *startSource = new LoginPage;
	connect(startSource, &LoginPage::permitted, this, &AppLoader::loadMainContent);
	setSource(startSource);
}

void AppLoader::loadMainContent()
{
	MainWindow *mainWindow = new MainWindow();
	//setSourceAnimated(mainWindow, Ui::AnimMode::Custom);
	setSourceAnimated(mainWindow);
}

void AppLoader::toggleFullScreen()
{
	isFullScreen() ? showNormal() : showFullScreen();
}
