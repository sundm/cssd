#include "printermanager.h"
#include <printer/labelprinter.h>

PrinterManager::PrinterManager() {

	_printer = PrinterFactory().Create(PrinterFactory::ZEBRA_GT8);

	_printer->setInversion(true);
}

PrinterManager::~PrinterManager() {
	_printer->close();
	delete _printer;
}

PrinterManager& PrinterManager::instance() {
	static PrinterManager instance;
	return instance;
}

LabelPrinter * PrinterManager::currentPrinter() {
	return instance()._printer;
}
