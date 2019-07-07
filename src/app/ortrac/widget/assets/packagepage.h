#pragma once

#include "ui/views.h"

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
};

QString literalSteType(int type);
QBrush brushForSteType(int type);

} //namespace Internal

class PackagePage : public QWidget
{
	Q_OBJECT

public:
	PackagePage(QWidget *parent = Q_NULLPTR);

private:
	void search();

	Internal::PackageAssetView *_view;
	SearchEdit *_searchBox;
};
