#pragma once

#include "ui/views.h"
#include <QTreeView>
#include "core/net/jsonhttpclient.h"
#include "rdao/entity/package.h"

class QStandardItemModel;

class PlateView : public TableView
{
	Q_OBJECT

public:
	PlateView(QWidget *parent = nullptr);
	bool hasPlate(int) const;
	void addPlate(const QString& udi);
	QVariantList plates() const;
	void clear();

private:
	QStandardItemModel * _model;
	JsonHttpClient _http;

	QList<Package *> _pkgList;
};


class PackPlateView : public QTreeView
{
	Q_OBJECT

public:
	PackPlateView(QWidget *parent = nullptr);
	void addPlate(int);
	QVariantList plates() const;
	void doPack(int, int);
	void clear();
//signals:
//	void packed(const std::list<std::list<std::string>> &);

private:
	enum {
		Package,
		PackType,
		Barcode,
		CardId,
		SterileDate,
		ExpireDate
	};

	QList<int> getSelectedPackages(QVariantList &) const;

	QStandardItemModel * _model;
	JsonHttpClient _http;
};
