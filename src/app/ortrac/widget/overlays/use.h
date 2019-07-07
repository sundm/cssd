#pragma once

#include "ui/loader.h"
#include <qscanner/qscanner.h>

class DeptEdit;
class PackageEdit;
class QSpinBox;
class TableView;
class OrRecyclePackageView;
class QStandardItemModel;

class UsePanel : public Ui::Source, public Scanable
{
	Q_OBJECT

public:
	UsePanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void addEntry();
	void removeEntry();
	void commit();

private:
	void updatePatient(const QString &);
	void reset();

	OrRecyclePackageView * _pkgView;
};
