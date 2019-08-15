#pragma once

#include <QGroupBox>
#include <qscanner/qscanner.h>
#include "core/net/jsonhttpclient.h"

class SearchEdit;
class QTextEdit;
class QFormLayout;
class QGridLayout;
class QStandardItemModel;
class QTableView;

class TracePaientItem : public QGroupBox
{
	Q_OBJECT

public:
	TracePaientItem(const QString &title = QString(), QWidget *parent = Q_NULLPTR);
	void addEntry(const QString &label, const QString &field);
private:
	QFormLayout * _formLayout;
};

class TracePatientPage : public QWidget, public Scanable, public JsonHttpClient
{
	Q_OBJECT

public:
	TracePatientPage(QWidget *parent = Q_NULLPTR);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void showDetail(const QModelIndex &);

private:
	void clear();
	void startTrace();
	void tracePackage(const QString &);
	void tracePatient(const QString &);
	
	SearchEdit *_searchBox;
	QGridLayout *_grid;

	QTableView *_view;
	QStandardItemModel * _model;
};
