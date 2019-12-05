#pragma once

#include <QString>

const QString commitStr = "910108";
const QString resetStr = "910201";

class Barcode
{
public:
	enum Type {
		Action,			// 91
		Commit,
		Reset,
		Package,		// 10
		User,			// 11
		Department,		// 12
		Device,			// 13
		Program,		// 14
		Instrument,		// 15
		Plate,			// 16
		Test,			// 20
		PkgCode,		// 30
		Unknown
	};

	Barcode(QString bc);
	
	int type() const;
	int intValue() const;

private:
	QString _bc;
};


class TranspondCode
{
public:
	enum Type {
		Package,		// E2009A9
		Instrument,		// E2009A8
		Unknown
	};

	TranspondCode(QString bc);

	int type() const;
private:
	QString _bc;
};