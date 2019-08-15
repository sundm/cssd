#pragma once

#include <QDialog>
#include <qscanner/qscanner.h>
#include "core/net/jsonhttpclient.h"

class SearchEdit;
class QLabel;
class QTextEdit;
class WaitingSpinner;

class WashAbnormal : public QDialog, public Scanable, public JsonHttpClient
{
    Q_OBJECT

public:
    WashAbnormal(Scanable* candidate = nullptr, QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;
	void accept() override;

private:
	void search(const QString &plate_id);
	void startTrace();

	WaitingSpinner *_waiter;
	SearchEdit *_searchBox;
	QLabel *_deviceName;
	QLabel *_deviceProgram;
	QLabel *_deviceCycle;
	QTextEdit *_reasonEdit;

	int _washId;
	bool _hasPlateId;
};
