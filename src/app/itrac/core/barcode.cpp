#include "Barcode.h"
#include <QRegExp>

Barcode::Barcode(QString bc)
	:_bc(bc)
{
}

int Barcode::type() const
{
	QRegExp re("\\d{6,}");
	if (!re.exactMatch(_bc))
		return Unknown;

	int len = _bc.length();
	QString indicator = _bc.left(2);
	if (_bc.compare(commitStr) == 0 )
		return Commit;
	if (_bc.compare(resetStr) == 0)
		return Reset;
	if ("10" == indicator && len>=16)
		return Package;
	if ("11" == indicator && 6 == len)
		return User;
	if ("12" == indicator && 8 == len)
		return Department;
	if ("13" == indicator && 8 == len)
		return Device;
	if ("14" == indicator && 8 == len)
		return Program;
	if ("15" == indicator && 8 == len)
		return Instrument;
	if ("16" == indicator && 8 == len)
		return Plate;
	if ("20" == indicator && 14 == len)
		return Test;
	if ("30" == indicator && 6 == len)
		return PkgCode;

	return Unknown;
}

int Barcode::intValue() const
{
	if (_bc.length() < 10)
		return _bc.toInt(); // return 0 if conversion fails
	return 0;
}

TranspondCode::TranspondCode(QString bc)
	:_bc(bc)
{
}

int TranspondCode::type() const
{
	int len = _bc.length();
	QString indicator = _bc.left(7);

	if ("E2009A9" == indicator && 24 == len)
		return Package;

	if ("E2009A8" == indicator && 24 == len)
		return Instrument;

	return Unknown;
}
