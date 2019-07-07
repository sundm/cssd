#pragma once

#include "ui/views.h"

class QStandardItemModel;

namespace Internal {

class InstrumentAssetView : public TableView
{
	Q_OBJECT

public:
	enum Column {Name, Id, Vip, Pinyin};
	InstrumentAssetView(QWidget *parent = nullptr);
	void load(int page = 0, int count = 10);

private:
	QStandardItemModel * _model;
};

QString getVipLiteral(const QString &vip);

} //namespace Internal

class InstrumentPage : public QWidget
{
	Q_OBJECT

public:
	InstrumentPage(QWidget *parent = Q_NULLPTR);

private slots:
	void refresh();
	void add();
	void modify();

private:
	Internal::InstrumentAssetView *_view;
};
