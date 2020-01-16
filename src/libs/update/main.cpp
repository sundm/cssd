#include "winsupdate.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	Winsupdate w;
	w.show();
	return a.exec();
}
