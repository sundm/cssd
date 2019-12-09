#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class SearchEdit;
class QPaginationWidget;

namespace Internal {

class PackageAssetView : public PaginationView
{
	Q_OBJECT

public:
	enum Column {Name, Pinyin, PackType, SteType, Department};
	PackageAssetView(QWidget *parent = nullptr);
	void load(const QString &kw = QString(), int page = 0);

private:
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

QString literalSteType(int type);
QBrush brushForSteType(int type);

} //namespace Internal

class PackagePage : public QWidget
{
	Q_OBJECT

public:
	PackagePage(QWidget *parent = Q_NULLPTR);

private slots:
	void reflash();
	void addEntry();
	void editEntry();
	//void infoEntry();
	void slotRowDoubleClicked(const QModelIndex &);

private:
	void doSearch(int page = 1);
	void search();
	void editRow(int row);

	Internal::PackageAssetView *_view;
	SearchEdit *_searchBox;
	QPaginationWidget *_paginator;
	JsonHttpClient _http;
};
