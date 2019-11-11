#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"
#include <QHash>

class QStandardItemModel;
class XPicture;
class QLabel;

struct instrument_struct
{
	QString name;
	QStringList codes;
};

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

class PackageInfoView : public QWidget
{
	Q_OBJECT
public:
	PackageInfoView(QWidget *parent = nullptr);
	void updateTips(const QString& tips);
	void updatePackageInfo(const QString &pkgId, const QString &pkgName, const int &insCount);
	void scanned();
	void unusualed();
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
	void loadDetail(const QHash<QString, QString>* const maps);
	void scanned(const QString & code);
	void clear();

signals:
	void sendData(int);

private slots:
	void slotItemClicked(const QModelIndex &);
	void updateState(int, int);

private:
	enum {Name, Total, Scanned, Residue, Tips};
	QStandardItemModel * _model;
	JsonHttpClient _http;

	QModelIndex posIndex;

	QList<instrument_struct> *_instruments;
	int _state;
};


