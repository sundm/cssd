#pragma once

class LabelPrinter;
class PrinterManager
{
private:
	PrinterManager();
	PrinterManager(const PrinterManager &) = delete;
	PrinterManager& operator=(const PrinterManager &) = delete;
	~PrinterManager();

	LabelPrinter *_printer;

public:
	static PrinterManager & instance();
	static LabelPrinter *currentPrinter();
};
