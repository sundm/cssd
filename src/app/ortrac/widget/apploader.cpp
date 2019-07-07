#include "apploader.h"
#include "widget/loginpage.h"
#include "widget/mainwindow.h"

AppLoader::AppLoader(QWidget *parent)
	: Ui::Loader(parent)
{
	Widget::LoginPage *startSource = new Widget::LoginPage;
	connect(startSource, &Widget::LoginPage::permitted, this, &AppLoader::loadMainContent);
	setSource(startSource);
}

void AppLoader::loadMainContent()
{
	MainWindow *mainWindow = new MainWindow();
	//setSourceAnimated(mainWindow, Ui::AnimMode::Custom);
	setSourceAnimated(mainWindow);
}
