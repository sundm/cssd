#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;

namespace Internal {

class InstrumentIdAssetView : public TableView
{
	Q_OBJECT

public:
	enum Column {Name, Id};
	InstrumentIdAssetView(QWidget *parent = nullptr);
	void load(int page = 0, int count = 10);

private:
	QStandardItemModel * _model;
	JsonHttpClient _http;
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
	void slotRowDoubleClicked(const QModelIndex &);

private:
	Internal::InstrumentIdAssetView *_view;
};
