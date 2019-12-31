#include "keygen.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	keygen w;
	w.show();
	return a.exec();
}
