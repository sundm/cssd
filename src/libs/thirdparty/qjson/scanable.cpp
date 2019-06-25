#include "scanable.h"
#include <qscanner/qscanner.h>

Scanable::Scanable() {
	QScannerFactory::connect([this](const QString &bc) {
		handleBarcode(bc);
	});
}

void Scanable::handleBarcode(const QString &)
{
}
