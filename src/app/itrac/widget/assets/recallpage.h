#pragma once

#include <QWidget>
#include "core/net/jsonhttpclient.h"

class QComboBox;
class TableView;
class QStandardItemModel;
class RecallPage : public QWidget, public JsonHttpClient
{
	Q_OBJECT

public:
	enum {PackageId, PackageName, PackType, ExpireDate};
	RecallPage(QWidget *parent = Q_NULLPTR);
	~RecallPage();

private slots:
	void onDeviceChanged(int);
	void recall();

private:
	void loadSterilizer();

	QComboBox * _comboBox;
	TableView * _view;
	QStandardItemModel *_model;
};
