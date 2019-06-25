#include "scanable.h"
#include <qscanner/qscanner.h>

Scanable::Scanable() {
}

void Scanable::bind(QObject *context) {
	QScannerFactory::connect(context, [this](const QString &bc) {
		handleBarcode(bc);
	});
}

void Scanable::handleBarcode(const QString &) {
}
