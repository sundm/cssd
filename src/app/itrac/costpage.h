#pragma once

#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;
class QStandardItemModel;
class TableView;
class Costpage : public QWidget
{
	Q_OBJECT

public:
	Costpage(QWidget *parent = Q_NULLPTR);
	~Costpage();

private slots:
	void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
	void initCostView();
	void updateDetailView(const QString& date);

	void refresh();
	void add();

private:
	QTreeWidget *_dateWidget;
	TableView *_detailView;
	QStandardItemModel * _detailModel;

	QString _dateString;
};
