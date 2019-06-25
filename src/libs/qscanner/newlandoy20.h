#ifndef NLOY20_H
#define NLOY20_H

#include "ibarcodegun.h"

class NewLandOY20 : public IBarcodeGun
{
	Q_OBJECT

public:
	NewLandOY20(QObject *parent = Q_NULLPTR);
	~NewLandOY20();

public:
	hid_device *open();
	QString format(unsigned char*);
};

#endif // !NLOY20_H