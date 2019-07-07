#pragma once

#include <QTableView>

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
};

} //namespace Internal

class DepartmentPage : public QWidget
{
	Q_OBJECT

public:
	DepartmentPage(QWidget *parent = Q_NULLPTR);
private:
	Internal::DeptAssetView *_view;
};
