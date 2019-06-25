#pragma once

#include <QThread>

struct hid_device_;
typedef struct hid_device_ hid_device;

class IBarcodeGun : public QThread
{
	Q_OBJECT

public:
	IBarcodeGun(QObject *parent = Q_NULLPTR);
	virtual ~IBarcodeGun();

	void stop(); // stop hid reading and quit the thread

protected:
	void run();
	virtual hid_device *open() = 0; // open the hid
	virtual QString format(unsigned char*) = 0; // format the barcode

signals:
	void dataRecieved(const QString &);

private:
	bool _scanning;
};
