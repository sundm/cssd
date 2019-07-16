#pragma once

#include <QTableView>
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;

namespace Internal {

class DeptAssetView : public QTableView
{
	Q_OBJECT

public:
	enum Column {Name, Id, Pinyin};
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
private:
	Internal::DeptAssetView *_view;
};
