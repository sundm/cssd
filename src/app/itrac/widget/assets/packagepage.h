#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class SearchEdit;

namespace Internal {

class PackageAssetView : public TableView
{
	Q_OBJECT

public:
	enum Column {Name, PackType, SteType, Pinyin, Department};
	PackageAssetView(QWidget *parent = nullptr);
	void load(const QString &kw = QString(), int page = 0, int count = 10);

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
	void infoEntry();
	void slotRowDoubleClicked(const QModelIndex &);

private:
	void search();
	void editRow(int row);

	Internal::PackageAssetView *_view;
	SearchEdit *_searchBox;
	JsonHttpClient _http;
};
