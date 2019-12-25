#pragma once

#include <QGroupBox>
#include "ui/views.h"

class SearchEdit;
class QTextEdit;
class QFormLayout;
class QGridLayout;
class QStandardItemModel;
class QTableView;
class QLabel;
class QSpinBox;
class QPushButton;
class PackageFlow;

class TracePackageInfoView : public QWidget
{
	Q_OBJECT
public:
	TracePackageInfoView(QWidget *parent = nullptr);
	void loadInfo(const QString &udi, bool isPackage = true);
signals:
	void packageFlow(const PackageFlow &);
private slots:
	void cycleChanged(int);
private:
	void init();
	void clear();
	void updateInfo();

	QLabel * _insNameLabel;
	QLabel * _pkgNameLabel;
	QLabel * _pkgUDILabel;
	QLabel * _deptLabel;
	QLabel * _totalCycleLabel;
	QSpinBox * _cyclelBox;
	QLabel * _patientLabel;
	QLabel * _surgeryLabel;

	QGridLayout *_grid;
	QString _udi;
	int _cycle;
	bool _isPackage;
};

class TraceDetailView : public TableView
{
	Q_OBJECT

public:
	TraceDetailView(QWidget *parent = nullptr);

public slots:
	void loadDetail(const PackageFlow &);

private:
	enum { Operate, OpName, OpTime, State };
	QStandardItemModel * _model;
};

class TracePackagePage : public QWidget
{
	Q_OBJECT

public:
	TracePackagePage(QWidget *parent = Q_NULLPTR);

private slots:
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);
private:
	void clear();
	void startTrace();
	
	SearchEdit *_searchBox;
	TracePackageInfoView *_infoView;
	TraceDetailView *_detailView;

	QString _udi;
};
