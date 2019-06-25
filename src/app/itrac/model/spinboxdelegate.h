#pragma once

#include <QItemDelegate>

class SpinBoxDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	SpinBoxDelegate(int column, int max, int min = 1, QObject *parent = 0);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const;

	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
	int _column;
	int _max;
	int _min;
};