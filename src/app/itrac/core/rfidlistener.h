#pragma once
#include <QObject>
#include <QString>
#include "../libs/rfidreader/rfidreader.h"

class RfidCodelistener : public QObject, public Abstractlistener
{
	Q_OBJECT
public:
	RfidCodelistener() {};
	virtual ~RfidCodelistener() {};
	virtual void actionperformed(CodeType type, std::string code);

signals:
	void onTransponder(const QString& code);
	void onBarcode(const QString& code);
};