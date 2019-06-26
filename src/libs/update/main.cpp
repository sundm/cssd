#include "winsupdate.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QString url = QString::fromLocal8Bit(argv[1]);
	Winsupdate w(url);
	w.show();
	return a.exec();
}
