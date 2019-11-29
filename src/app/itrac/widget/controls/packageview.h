#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"
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
private:
	enum { Barcode, Name, PackType, Department, ExpireDate, SterType, Implant};
};

class SterileCheckPackageView : public TableView
{
	Q_OBJECT

public:
	SterileCheckPackageView(QWidget *parent = nullptr);
	void addPackage(const QString &, const QString &, const bool &);

private:
	enum {Barcode, Name, Implant};
	QStandardItemModel *_model;
};

class DispatchPackageView : public AbstractPackageView
{
	Q_OBJECT

public:
	DispatchPackageView(QWidget *parent = nullptr);
	void addPackage(const QString &) override;
	
private:
	enum { Barcode, Name, PackType, Department, ExpireDate, Implant};
};

class OperationInfoTabelView : public TableView
{
	Q_OBJECT

public:
	OperationInfoTabelView(QWidget *parent = nullptr);
	void loadOperations();

signals:
	void operationClicked(const QString &operationId);

private slots:
	void slotRowDoubleClicked(const QModelIndex &);

private:
	enum { OperationID, OperationRoom, OperationTime, OperationName, PatientName };
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

class OperationInfoView : public QWidget
{
	Q_OBJECT

public:
	OperationInfoView(QWidget *parent = nullptr);

signals:
	void operation(const QString &operationId);

private slots:
	void addOperation();
	void delOperation();
	void refresh();

private:
	OperationInfoTabelView * _view;
};

class OperationPackageView : public TableView
{
	Q_OBJECT

public:
	OperationPackageView(QWidget *parent = nullptr);
	void loadPackages(const QString& operationId);

private:
	enum { PackageType, PackageID, PackageName, State};
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

class PackageSimpleInfoView : public QWidget
{
	Q_OBJECT
public:
	PackageSimpleInfoView(QWidget *parent = nullptr);
	void updatePackageInfo(const int &insCount);
	void scanned();
	void unusualed();
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


