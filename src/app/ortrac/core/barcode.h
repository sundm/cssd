#pragma once

#include <QString>

class Barcode
{
public:
	enum Type {
		Action,       // 91
		Package,      // 10
		User,         // 11
		Department,   // 12
		Device,       // 13
		Program,      // 14
		Instrument,   // 15
		Plate,        // 16
		Test,         // 20
		Unknown
	};

	Barcode(QString bc);
	
	int type() const;
	int intValue() const;

private:
	QString _bc;
};


