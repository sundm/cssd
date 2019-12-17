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

class TracePackageInfoView : public QWidget
{
	Q_OBJECT
public:
	TracePackageInfoView(QWidget *parent = nullptr);
	void loadInfo(const QString &udi, const int &cycel, bool isPackage = true);
signals:
	void cycle(int);
private slots:
	void cycleChanged(int);
private:
	void init();
	void clear();
	void updateInfo();

	QLabel * _insNameLabel;
	QLabel * _pkgNameLabel;
	QLabel * _deptLabel;
	QLabel * _insNumLabel;
	QLabel * _totalCycleabel;
	QSpinBox * _cyclelBox;
	QLabel * _patientLabel;

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
	void loadDetail(const QString& udi, const int cycle);
private slots:
	void clickButton();
private:
	enum { Operate, OpName, OpTime, State };
	QStandardItemModel * _model;
	QPushButton* _btn;
};

class TracePackagePage : public QWidget
{
	Q_OBJECT

public:
	TracePackagePage(QWidget *parent = Q_NULLPTR);

private slots:
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);
	void onCycleChange(int);
private:
	void clear();
	void startTrace();
	void tracePackage(const QString &);
	
	SearchEdit *_searchBox;
	TracePackageInfoView *_infoView;
	TraceDetailView *_detailView;

	QString _udi;
};
