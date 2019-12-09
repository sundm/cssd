#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class SearchEdit;
class QPaginationWidget;

namespace Internal {

class PackageIdAssetView : public PaginationView
{
	Q_OBJECT

public:
	enum Column {Name, Id};
	PackageIdAssetView(QWidget *parent = nullptr);
	void load(const QString &kw = QString(), int page = 0);

private:
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

} //namespace Internal

class PackageIdPage : public QWidget
{
	Q_OBJECT

public:
	PackageIdPage(QWidget *parent = Q_NULLPTR);

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

	Internal::PackageIdAssetView *_view;
	SearchEdit *_searchBox;
	QPaginationWidget *_paginator;
	JsonHttpClient _http;
};
