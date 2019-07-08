#pragma once

#include "ui/views.h"

class QStandardItemModel;

class PackageView : public TableView
{
	Q_OBJECT

public:
	enum { Barcode, Name, PackType, Department, ExpireDate};
	PackageView(QWidget *parent = nullptr);

	bool hasPackage(const QString &) const;
	QVariantList packages() const;
	void addPackage(const QString &);

private:
	QStandardItemModel * _model;
};




