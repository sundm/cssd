#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class SearchEdit;
class QPaginationWidget;

namespace Internal {

class InstrumentAssetView : public PaginationView
{
	Q_OBJECT

public:
	enum Column {Name, Pinyin, Vip, Implant};
	InstrumentAssetView(QWidget *parent = nullptr);
	void load(const QString &kw = QString(), int page = 0);

private:
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

} //namespace Internal

class InstrumentPage : public QWidget, public JsonHttpClient
{
	Q_OBJECT

public:
	InstrumentPage(QWidget *parent = Q_NULLPTR);

private slots:
	void refresh();
	void add();
	void modify();
	void slotRowDoubleClicked(const QModelIndex &);
	void doSearch(int page = 1);
	void search();
private:
	Internal::InstrumentAssetView *_view;
	SearchEdit *_searchBox;
	QPaginationWidget *_paginator;
};
