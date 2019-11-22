#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class SearchEdit;

namespace Internal {

class PackageIdAssetView : public TableView
{
	Q_OBJECT

public:
	enum Column {Name, Id, Basics};
	PackageIdAssetView(QWidget *parent = nullptr);
	void load(const QString &kw = QString(), int page = 0, int count = 10);

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
	void infoEntry();
	void slotRowDoubleClicked(const QModelIndex &);

private:
	void search();
	void editRow(int row);

	Internal::PackageIdAssetView *_view;
	SearchEdit *_searchBox;
	JsonHttpClient _http;
};
