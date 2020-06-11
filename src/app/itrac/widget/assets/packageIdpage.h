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
	enum Column {Name, Id, Alias, Basics, Cycle};
	PackageIdAssetView(QWidget *parent = nullptr);
	void load(const QString &kw = QString(), int page = 0);
	int totalCount() { return _total; };
	int pageCount() { return _pageCount; }
private:
	QStandardItemModel * _model;
	JsonHttpClient _http;
	int _total;
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
	void delEntry();
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
