#pragma once

#include "ui/views.h"
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;

namespace Internal {

class InstrumentAssetView : public TableView
{
	Q_OBJECT

public:
	enum Column {Name, Id, Vip, Implant, Pinyin};
	InstrumentAssetView(QWidget *parent = nullptr);
	void load(int page = 0, int count = 10);

private:
	QStandardItemModel * _model;
	JsonHttpClient _http;
};

QString getVipLiteral(const QString &vip);
QString getImportLiteral(const QString &import);

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

private:
	Internal::InstrumentAssetView *_view;
};
