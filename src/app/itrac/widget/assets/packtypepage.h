#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class SearchEdit;

namespace Internal {

class PacktypeAssetView : public TableView
{
	Q_OBJECT

public:
	enum Column {PackType, Valid, Max};
	PacktypeAssetView(QWidget *parent = nullptr);
	void load();

private:
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

QString literalSteType(int type);
QBrush brushForSteType(int type);

} //namespace Internal

class PacktypePage : public QWidget
{
	Q_OBJECT

public:
	PacktypePage(QWidget *parent = Q_NULLPTR);

private slots:
	void reload();
	void addEntry();
	void modify();

private:
	Internal::PacktypeAssetView *_view;
};
