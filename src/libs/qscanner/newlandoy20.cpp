#include <QDebug>
#include <thirdparty/hidapi/hidapi/hidapi.h>
#include "newlandoy20.h"

NewLandOY20::NewLandOY20(QObject *parent)
	: IBarcodeGun(parent)
{
}

NewLandOY20::~NewLandOY20()
{
}

hid_device * NewLandOY20::open()
{
	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
	return hid_open(0x1EAB, 0x8310, NULL);
}

QString NewLandOY20::format(unsigned char *buf)
{
	if (buf[0] != 0x02) {
		//read data is not from OY20
		return QString();
	}

	int length = buf[1];
	buf[2 + length] = 0;
	return QString((const char *)(buf+2));
}