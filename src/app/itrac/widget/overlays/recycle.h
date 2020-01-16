#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"
#include "rdao/entity/package.h"
#include <QHash>
#include <QNetworkReply>

class DeptEdit;
class PackageEdit;
class QSpinBox;
class TableView;
class XPicture;
class UnusualInstrumentView;
class PackageInfoView;
class PackageDetailView;
class QStandardItemModel;
class FtpManager;

class NoBCRecyclePanel : public CssdOverlayPanel, public JsonHttpClient
{
	Q_OBJECT

public:
	NoBCRecyclePanel(QWidget *parent = nullptr);
	
	bool accept() override;

protected:
	void handleBarcode(const QString &) override;

private slots:
	void addEntry();
	void removeEntry();
	void addPlate();
	void commit();
	void loadPkg(int);

private:
	void initPackageView();
	int findRow(int deptId, int pkgId);
	void updatePlate(const QString &);
	void updateDept(const QString &);
	void reset();

	DeptEdit * _deptEdit;
	PackageEdit *_pkgEdit;
	TableView * _pkgView;
	QStandardItemModel *_pkgModel;
};

class OrRecyclePanel : public CssdOverlayPanel, public JsonHttpClient
{
	Q_OBJECT

public:
	OrRecyclePanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
	void reset();

	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);
	
	void onScanned(const QString& code);
	void onUnusual(const QString& code);
	void loadInstrumentImg(const QString& udi);
	void imgInsClicked();
	void imgPkgClicked();

	void imgError(QNetworkReply::NetworkError);
	void imgLoaded();
private:
	const QString getFileMd5(const QString &filePath);
	void loadPackageImg(const QString& udi);

	PackageInfoView * _pkgView;
	PackageDetailView * _detailView;
	UnusualInstrumentView * _unusualView;

	QStringList * _scannedCodes;
	QStringList * _unusualCodes;
	Package _package;
	FtpManager * _ftp;
	XPicture * _pkgImg;
	XPicture * _insImg;
	int _row;
	int _step;
	int _imgType;
	QString _imgFilePath;
};