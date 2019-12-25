#pragma once

#include "ui/views.h"
#include <QTreeView>

class SearchEdit;
class QTextEdit;
class QFormLayout;
class QGridLayout;
class QStandardItemModel;
class QTableView;
class QLabel;

class SurgeryView : public QTreeView
{
	Q_OBJECT

public:
	SurgeryView(QWidget *parent = nullptr);
	void addPatient(const QString& patientId);
	void clear();

signals:
	void packageClicked(const QString&, const int);

private slots:
	void onClicked(const QModelIndex &);

private:
	enum {
		Patient,
		Surgeries,
		Packages
	};

	QStandardItemModel * _model;
};

class PatientPackageInfoView : public QWidget
{
	Q_OBJECT
public:
	PatientPackageInfoView(QWidget *parent = nullptr);
	void clear();
public slots:
	void loadInfo(const QString &, const int);

private:
	QLabel * _pkgNameLabel;
	QLabel * _pkgUDILabel;
	QLabel * _deptLabel;
	QLabel * _insNumLabel;
	QLabel * _totalCycleLabel;
	QLabel * _cyclelLabel;
};

class PatientPackageDetailView : public TableView
{
	Q_OBJECT

public:
	PatientPackageDetailView(QWidget *parent = nullptr);
	void clear();
public slots:
	void loadDetail(const QString&, const int);

private:
	enum { Operate, OpName, OpTime, State };
	QStandardItemModel * _model;
};

class TracePatientPage : public QWidget
{
	Q_OBJECT

public:
	TracePatientPage(QWidget *parent = Q_NULLPTR);

private slots:
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);

private:
	void clear();
	void startTrace();
	void tracePatient(const QString &);
	
	SearchEdit *_searchBox;

	SurgeryView *_surgeryView;
	PatientPackageInfoView *_pkgInfoView;
	PatientPackageDetailView *_pkgDetailView;
};
