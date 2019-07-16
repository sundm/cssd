#pragma once

#include <QGroupBox>
#include <qscanner/qscanner.h>
#include "core/net/jsonhttpclient.h"

class SearchEdit;
class QTextEdit;
class QFormLayout;
class QGridLayout;

class TraceItem : public QGroupBox
{
	Q_OBJECT

public:
	TraceItem(const QString &title = QString(), QWidget *parent = Q_NULLPTR);
	void addEntry(const QString &label, const QString &field);
private:
	QFormLayout * _formLayout;
};

class TracePage : public QWidget, public Scanable, public JsonHttpClient
{
	Q_OBJECT

public:
	TracePage(QWidget *parent = Q_NULLPTR);

protected:
	void handleBarcode(const QString &) override;

private:
	void clear();
	void startTrace();
	void tracePackage(const QString &);
	void tracePatient(const QString &);
	
	SearchEdit *_searchBox;
	QGridLayout *_grid;
};
