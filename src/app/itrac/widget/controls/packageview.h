#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"
#include "rdao/entity/package.h"
#include "rdao/entity/device.h"
#include "rdao/entity/surgery.h"
#include <QHash>

class QStandardItemModel;
class XPicture;
class QLabel;
struct Instrument;
namespace Ui {
	class FlatEdit;
}

class AbstractPackageView : public TableView
{
	Q_OBJECT

public:
	AbstractPackageView(QWidget *parent = nullptr);
	bool hasPackage(const QString &) const;
	bool hasImplantPackage() const;
	QVariantList packageIds() const;
	QVariantList cardIds() const;

	virtual void addPackage(const QString &) = 0;

protected:
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

class OrRecyclePackageView : public AbstractPackageView
{
	Q_OBJECT

public:
	enum { Barcode, PkgCode, Name, PackType, Department, ExpireDate, VPlate };
	OrRecyclePackageView(QWidget *parent = nullptr);
	void addExtPackage(const QString&, const QString&, const QString&);
	void addPackage(const QString &) override;
	void updatePlate(const QString &);
	int plate() const;
};

class SterilePackageView : public AbstractPackageView
{
	Q_OBJECT

public:
	SterilePackageView(QWidget *parent = nullptr);
	void addPackage(const QString &) override;
	bool matchType(int type) const;
	const QList<Package> & packages() const;
	void clear();
private:
	QList<Package> _pkgList;
	enum { Barcode, Name, PackType, Department, ExpireDate, SterType, Implant};
};

class SterileCheckPackageView : public TableView
{
	Q_OBJECT

public:
	SterileCheckPackageView(QWidget *parent = nullptr);
	void addPackages(const QList<SterilizeResult::PackageItem> &packages, const bool readOnly = false);
	const QList<SterilizeResult::PackageItem> & getPackages();

private slots:
	void itemChecked(const QModelIndex &, const bool);

private:
	enum {Barcode, Name, Implant, Wet};
	QStandardItemModel *_model;
	QList<SterilizeResult::PackageItem> _packages;
};

class DispatchPackageView : public AbstractPackageView
{
	Q_OBJECT

public:
	DispatchPackageView(QWidget *parent = nullptr);
	void addPackage(const QString &) override;
	const QList<Package> & packages() const;
	void clear();
private:
	QList<Package> _pkgList;
	enum { Barcode, Name, PackType, Department, ExpireDate, Implant};
};

class OperationInfoTabelView : public TableView
{
	Q_OBJECT

public:
	OperationInfoTabelView(QWidget *parent = nullptr);
	void loadSurgeries(Rt::SurgeryStatus status);

signals:
	void operationClicked(const int);

private slots:
	void slotRowDoubleClicked(const QModelIndex &);

private:
	enum { OperationID, OperationRoom, OperationTime, OperationName, PatientId, PatientName };
	QStandardItemModel * _model;
	QList<Surgery> _surgeries;
};

class OperationInfoView : public QWidget
{
	Q_OBJECT

public:
	OperationInfoView(QWidget *parent = nullptr);
	void loadSurgeries();
signals:
	void operation(const int);

public slots:
	void refresh();

private slots:
	void addOperation();
	void delOperation();
	

private:
	OperationInfoTabelView * _view;
	
};

class OperationCheckPackageView : public TableView
{
	Q_OBJECT

public:
	OperationCheckPackageView(QWidget *parent = nullptr);
	Surgery& getSurgery() { return _surgery; }
	void setScanned(const QString& udi);
signals:
	void waitForScan(const QList<Package> &);

public slots:
	void loadPackages(const int);
private:
	enum {PackageID, PackageName, State, Info};
	QStandardItemModel * _model;
	Surgery _surgery;
	QList<Package> _packages;
};

class OperationPackageView : public TableView
{
	Q_OBJECT

public:
	OperationPackageView(QWidget *parent = nullptr);
	bool isFinished();
	bool addPackage(const Package &pkg);
	Surgery& getSurgery() { return _surgery; }
public slots:
	void loadPackages(const int);

private:
	enum { PackageType, PackageID, PackageName, State};
	QStandardItemModel * _model;
	Surgery _surgery;
	QList<Package> _packages;
};

class PackageSimpleInfoView : public QWidget
{
	Q_OBJECT
public:
	PackageSimpleInfoView(QWidget *parent = nullptr);
	void reset();
	void updatePackageInfo(const int &insCount);
	void scanned();
	void unusualed();
	bool isScanFinished();
private:
	QLabel * _totalNumLabel;
	QLabel * _scannedNumLabel;
	QLabel * _residueNumLabel;
	QLabel * _unusualNumLabel;

	JsonHttpClient _http;

	int _totalNum;
	int _scannedNum;
	int _unusualNum;
};

class PackageInfoView : public QWidget
{
	Q_OBJECT
public:
	PackageInfoView(QWidget *parent = nullptr);
	void reset();
	void updateTips(const QString& tips);
	void updatePackageInfo(const QString &pkgId, const QString &pkgName, const int &insCount);
	void scanned();
	void unusualed();
	bool isScanFinished();
private:
	QLabel * _tipsLabel;

	QLabel * _packageIDLabel;
	QLabel * _packageNameLabel;

	QLabel * _totalNumLabel;
	QLabel * _scannedNumLabel;
	QLabel * _residueNumLabel;
	QLabel * _unusualNumLabel;

	JsonHttpClient _http;

	int _totalNum;
	int _scannedNum;
	int _unusualNum;
};

class UnusualInstrumentView : public TableView
{
	Q_OBJECT

public:
	UnusualInstrumentView(QWidget *parent = nullptr);
	void addUnusual(const QString& instrumentID);	
	void reset();

private:
	enum {InstrumentID, InstrumentName, PackageID, PackageName};
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

class PackageDetailView : public TableView
{
	Q_OBJECT

public:
	PackageDetailView(QWidget *parent = nullptr);
	void loadDetail(const QList<Instrument> *instruments);
	void scanned(const QString & code);
	void reset();
signals:
	void scand(const QString &);
	void unusual(const QString &);
	void onclick(const QString &);

private slots:
	void slotItemClicked(const QModelIndex &);

private:
	enum {Name, Code, Status};
	QStandardItemModel * _model;
	JsonHttpClient _http;

	QModelIndex posIndex;

	QList<Instrument> *_instruments;
};


