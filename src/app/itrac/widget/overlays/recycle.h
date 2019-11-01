#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"

class DeptEdit;
class PackageEdit;
class QSpinBox;
class TableView;
class OrRecyclePackageView;
class PackageDetailView;
class QStandardItemModel;

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
	void addEntry();
	void addPlateEntry();
	void removeEntry();
	void chooseExt();
	void commit();
	void setExtPkg(const QString&, const QString&, const QString&);
	void showDetail(const QModelIndex &index);
	void updateRecord(int pkg_record);
	
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);

private:
	void updatePlate(const QString &);
	void reset();

	OrRecyclePackageView * _pkgView;
	PackageDetailView * _detailView;

	int _row;
};
