#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"
#include "rdao/entity/surgery.h"
#include <QHash>

class DeptEdit;
class PackageEdit;
class QSpinBox;
class TableView;
class XPicture;
class OperationInfoTabelView;
class OperationInfoView;
class OperationPackageView;
class OperationCheckPackageView;
class UnusualInstrumentView;
class PackageSimpleInfoView;
class PackageInfoView;
class PackageDetailView;
class QStandardItemModel;

class PreBindPanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	PreBindPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
	void reset();
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);

	void onScanned(const QString& code);
	void onUnusual(const QString& code);
	void loadPackage(const int);
private:
	void initOperationView();

	OperationInfoView * _operInfoView;
	OperationPackageView * _operPackageView;

	QStringList * _scannedCodes;
	QStringList * _unusualCodes;

	int _row;
};


class PreExamPanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	PreExamPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
	void reset();
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);

	void onScanned(const QString& code);
	void onUnusual(const QString& code);
	void loadPackage(const int);
	void loadInsturments(const QList<Package>& pkgs);
private:
	void initOperationView();
	
	OperationInfoTabelView * _operInfoView;
	OperationCheckPackageView * _operPackageView;

	PackageSimpleInfoView * _pkgView;
	PackageDetailView * _detailView;
	UnusualInstrumentView * _unusualView;

	QStringList * _scannedCodes;
	QStringList * _unusualCodes;
	QList<Package> _packages;
	int _row;
	int _step;
};

class PostExamPanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	PostExamPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
	void reset();
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);

private:
	void initOperationView();

	OperationInfoTabelView * _operInfoTableView;
	OperationCheckPackageView * _operPackageView;
	PackageSimpleInfoView * _pkgView;
	PackageDetailView * _detailView;
	UnusualInstrumentView * _unusualView;

	QHash<QString, QString> * _codeMap;
	QStringList * _scannedCodes;
	QStringList * _unusualCodes;

	int _row;
	int _step;
};