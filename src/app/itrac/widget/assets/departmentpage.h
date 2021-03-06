#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;

namespace Internal {

class DeptAssetView : public TableView
{
	Q_OBJECT

public:
	enum Column {Name, Id, Pinyin, Phone};
	DeptAssetView(QWidget *parent = nullptr);
	void load(int page = 0, int count = 10);

private:
	void clear();
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

} //namespace Internal

class DepartmentPage : public QWidget, public JsonHttpClient
{
	Q_OBJECT

public:
	DepartmentPage(QWidget *parent = Q_NULLPTR);

private slots:
	void addEntry();
	void modify();

private:
	Internal::DeptAssetView *_view;
};
