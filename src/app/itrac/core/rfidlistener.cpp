#include "rfidlistener.h"
#include <QDebug>
using namespace std;

void RfidCodelistener::actionperformed(CodeType type, string code)
{
	if (type == CodeType::Barcode) {
		qDebug() << "Get Barcode." << endl;
		emit onBarcode(QString::fromStdString(code));
	}
	else if (type == CodeType::Transponder) {
		qDebug() << "Get Transponder." << endl;
		emit onTransponder(QString::fromStdString(code));
	}
}