#pragma once

#include <QDialog>

class SearchEdit;
class QTableView;
class QStandardItemModel;
class QSortFilterProxyModel;
class DeptPicker : public QDialog
{
	Q_OBJECT

public:
	DeptPicker(QWidget *parent = 0);
	~DeptPicker();

signals:
	void deptPicked(int id, const QString &name);

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private:
	void acceptCurrentEntry();

private slots:
	void acceptEntry(const QModelIndex &index);

private:
	SearchEdit * _searchEdit;
	QTableView *_deptView;
	QStandardItemModel *_deptModel;
	QSortFilterProxyModel *_deptProxyModel;
};
