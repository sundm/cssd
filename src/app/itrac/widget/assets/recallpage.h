#pragma once

#include <QWidget>

class QComboBox;
class TableView;
class QStandardItemModel;
class RecallPage : public QWidget
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
