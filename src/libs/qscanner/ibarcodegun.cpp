#include "ibarcodegun.h"
#include <thirdparty/hidapi/hidapi/hidapi.h>

#define MAX_STR 255

IBarcodeGun::IBarcodeGun(QObject *parent)
	: QThread(parent)
	, _scanning(false)
{
}

IBarcodeGun::~IBarcodeGun()
{
}

void IBarcodeGun::stop()
{
	_scanning = false;
}

void IBarcodeGun::run()
{
	_scanning = true;
	int res = -1;
	unsigned char buf[MAX_STR] = { 0 };

	if (hid_init() != 0)
		return;

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	hid_device *handle = open();
	if (!handle) return;

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);

	// Read requested state. hid_read() has been set to be
	// non-blocking by the call to hid_set_nonblocking() above.
	// This loop demonstrates the non-blocking nature of hid_read().
	while (_scanning) {
		res = hid_read(handle, buf, sizeof(buf)); // FIXME: buf size enough?

		if (res == 0) { // waiting read
			//printf("waiting...\n");
		}

		if (res < 0) { // read error
			// printf("Unable to read()\n");
		}

		if (res > 0) {
			QString barcode = format(buf);
			if (!barcode.isEmpty()) {
				// do not call handler directly, if network operations involved, they fail
				//if (_s) _s->handleBarcode(QString((const char*)out_data));

				// instead, emit it to a receiver who lives in the main thread
				emit dataRecieved(barcode);
			}
		}
		msleep(100);
	}

	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();
}
