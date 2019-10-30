#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class XPicture;

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

class PackageDetailView : public QWidget
{
	Q_OBJECT

public:
	PackageDetailView(QWidget *parent = nullptr);
	void loadDetail(const QString& pkgId, const QString& pkgTypeId, const QString& cardId);
	void clear();

signals:
	void sendData(int);

private slots:
	void imgClicked();
	void regist();
	void slotItemDoubleClicked(const QModelIndex &);
	void showContextMenu(const QPoint&);
	void updateState(int, int);
private:
	void imgLoad(const QString& pkgTypeId);

	enum {Name, Number, State};
	QTableView* _view;
	QStandardItemModel* _model;
	XPicture* _imgLabel;
	JsonHttpClient _http;

	QModelIndex posIndex;

	QString _pkg_id;
	QString _card_id;
	QString _ins_name;
	QString _ins_id;
	int _state;
};


