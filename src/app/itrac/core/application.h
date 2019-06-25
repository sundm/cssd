#pragma once

#include <QApplication>

class MainWindow;
class WaitingSpinner;

namespace Ui {
	class Loader;
}

namespace Core {

class Launcher;
struct User;

class Application : public QApplication
{
	Q_OBJECT

public:
	Application(Core::Launcher* launcher, int &argc, char **argv);
	~Application();

	void updateStyle();
	void startWaitingOn(QWidget*);
	void stopWaiting();

	const Ui::Loader *loader() const;
	MainWindow *mainWindow() const;
	User &currentUser() const; 

private:
	void setupScanner();

	Core::Launcher *_launcher;
	std::unique_ptr<Ui::Loader> _loader;
	std::unique_ptr<User> _op;
	std::unique_ptr<WaitingSpinner> _waiter;
};

Application *app();

} // namespace Core
