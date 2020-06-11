#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class SearchEdit;
class QPaginationWidget;

namespace Internal {

class InstrumentIdAssetView : public PaginationView
{
	Q_OBJECT

public:
	enum Column {Name, Id, Alias, Basics};
	InstrumentIdAssetView(QWidget *parent = nullptr);
	void load(const QString &kw = QString(), int page = 0);
	int totalCount() { return _total; };
	int pageCount() { return _pageCount; }
private:
	QStandardItemModel * _model;
	JsonHttpClient _http;
	int _total;
};

} //namespace Internal

class InstrumentIdPage : public QWidget, public JsonHttpClient
{
	Q_OBJECT

public:
	InstrumentIdPage(QWidget *parent = Q_NULLPTR);

private slots:
	void refresh();
	void add();
	void modify();
	void del();
	void slotRowDoubleClicked(const QModelIndex &);
	void doSearch(int page = 1);
	void search();
private:
	Internal::InstrumentIdAssetView *_view;
	SearchEdit *_searchBox;
	QPaginationWidget *_paginator;
};
