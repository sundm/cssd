#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class ClickedLabel;

class AbstractPackageView : public TableView
{
	Q_OBJECT

public:
	AbstractPackageView(QWidget *parent = nullptr);
	bool hasPackage(const QString &) const;
	QVariantList packages() const;

	virtual void addPackage(const QString &) = 0;

protected:
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

class OrRecyclePackageView : public AbstractPackageView
{
	Q_OBJECT

public:
	enum { Barcode, Name, PackType, Department, ExpireDate, VPlate };
	OrRecyclePackageView(QWidget *parent = nullptr);
	void addExtPackage(const QString&, const QString&);
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

private:
	enum { Barcode, Name, PackType, Department, ExpireDate };
};

class SterileCheckPackageView : public TableView
{
	Q_OBJECT

public:
	SterileCheckPackageView(QWidget *parent = nullptr);
	void addPackage(const QString &, const QString &);

private:
	enum { Barcode, Name };
	QStandardItemModel *_model;
};

class DispatchPackageView : public AbstractPackageView
{
	Q_OBJECT

public:
	DispatchPackageView(QWidget *parent = nullptr);
	void addPackage(const QString &) override;

private:
	enum { Barcode, Name, PackType, Department, ExpireDate };
};

class PackageDetailView : public QWidget
{
	Q_OBJECT

public:
	PackageDetailView(QWidget *parent = nullptr);
	void loadDetail(const QString& pkgTypeId);
	void clear();

private slots:
	void imgClicked();

private:
	void imgLoad(const QString& pkgTypeId);

	enum {Name, Number};
	QTableView* _view;
	QStandardItemModel* _model;
	ClickedLabel* _imgLabel;

	JsonHttpClient _http;

	QString _imgFileName;
};


