#pragma once

#include <QDialog>

class SearchEdit;
class QListView;
class QAbstractItemModel;
class QSortFilterProxyModel;
class IdPicker : public QDialog
{
	Q_OBJECT

public:
	IdPicker(QAbstractItemModel *srcModel, QWidget *parent = Q_NULLPTR);

signals:
	void idPicked(int id, const QString &name);

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private:
	void acceptCurrentEntry();

private slots:
	void acceptEntry(const QModelIndex &index);

private:
	SearchEdit * _searchEdit;
	QListView *_view;
	QSortFilterProxyModel *_proxyModel;
};
