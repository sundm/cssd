
#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QItemDelegate>
#include <QStyledItemDelegate>

/*
 * ReadOnly
 */
class ReadOnlyDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	ReadOnlyDelegate(QObject *parent = 0) : QItemDelegate(parent) {}

	void setItems(QStringList items);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex	&index) const
	{
		return NULL;
	}
};

/*
 * ComboBox
 */
class ComboDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	ComboDelegate(QObject *parent = 0);

	void setItems(QStringList items);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex	&index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const  QModelIndex &index) const;

private:
	QStringList m_sItemList;
};

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

/*
 * CheckBox
 */
class CheckBoxDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	CheckBoxDelegate(QObject *parent = 0);

	void setColumn(int col, bool readOnly = false);

signals:
	void setChecked(const QModelIndex &, const bool);

protected:
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
	int column;         //设置复选框的列
	bool _readOnly;
};

#endif // ITEMDELEGATE_H