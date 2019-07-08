#pragma once

#include "cssd_overlay.h"

class DeptEdit;
class PackageEdit;
class QSpinBox;
class TableView;
class OrRecyclePackageView;
class QStandardItemModel;

class NoBCRecyclePanel : public CssdOverlayPanel
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
	void commit();

private:
	void initPackageView();
	int findRow(int deptId, int pkgId);
	void updatePlate(const QString &);
	void reset();

	DeptEdit * _deptEdit;
	PackageEdit *_pkgEdit;
	TableView * _pkgView;
	QStandardItemModel *_pkgModel;
};

class OrRecyclePanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	OrRecyclePanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void addEntry();
	void removeEntry();
	void choosePlate();
	void commit();

private:
	void updatePlate(const QString &);
	void reset();

	OrRecyclePackageView * _pkgView;
};
