
#include "itemdelegate.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QDebug>

/*
 * ComboDelegate
 */
ComboDelegate::ComboDelegate(QObject *parent) :
	QItemDelegate(parent)
{
}

//自定义下拉框列表项
void ComboDelegate::setItems(QStringList items)
{
	m_sItemList = items;
}

QWidget *ComboDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	QComboBox *editor = new QComboBox(parent);
	editor->addItems(m_sItemList);
	editor->installEventFilter(const_cast<ComboDelegate*>(this));
	return editor;
}

void ComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QString str = index.model()->data(index).toString();

	QComboBox *box = static_cast<QComboBox*>(editor);
	int i = box->findText(str);
	box->setCurrentIndex(i);
}

void ComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QComboBox *box = static_cast<QComboBox*>(editor);
	QString str = box->currentText();
	model->setData(index, str);
}

void ComboDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
	editor->setGeometry(option.rect);
}

/*
 * SpinBoxDelegate
 */
SpinBoxDelegate::SpinBoxDelegate(int column, int max, int min, QObject *parent)
	: QItemDelegate(parent)
	, _column(column)
	, _max(max)
	, _min(min) {
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
	const QStyleOptionViewItem &/* option */,
	const QModelIndex & index) const {

	if (index.column() != _column)
		return nullptr;

	QSpinBox *editor = new QSpinBox(parent);
	editor->setMinimum(_min);
	editor->setMaximum(_max);

	return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
	const QModelIndex &index) const
{
	int value = index.model()->data(index, Qt::EditRole).toInt();

	QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
	spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
	const QModelIndex &index) const
{
	QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
	spinBox->interpretText();
	int value = spinBox->value();

	model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
	const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}

/*
 * CheckBoxDelegate
 */
static QRect CheckBoxRect(const QStyleOptionViewItem &viewItemStyleOptions)/*const*/
{
	//绘制按钮所需要的参数
	QStyleOptionButton checkBoxStyleOption;
	//按照给定的风格参数 返回元素子区域
	QRect checkBoxRect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkBoxStyleOption);
	//返回QCheckBox坐标
	QPoint checkBoxPoint(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width() / 2 - checkBoxRect.width() / 2,
		viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - checkBoxRect.height() / 2);
	//返回QCheckBox几何形状
	return QRect(checkBoxPoint, checkBoxRect.size());
}

CheckBoxDelegate::CheckBoxDelegate(QObject *parent) :
	QStyledItemDelegate(parent)
{

}

void CheckBoxDelegate::setColumn(int col, bool readOnly)
{
	column = col;
	_readOnly = readOnly;
}

// 绘制复选框
void CheckBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)const
{
	bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

	if (index.column() == column) {
		QStyleOptionButton checkBoxStyleOption;
		checkBoxStyleOption.state |= QStyle::State_Enabled;
		checkBoxStyleOption.state |= checked ? QStyle::State_On : QStyle::State_Off;
		checkBoxStyleOption.rect = CheckBoxRect(option);

		QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxStyleOption, painter);
	}
	else {
		QStyledItemDelegate::paint(painter, option, index);
	}
}

// 响应鼠标事件，更新数据
bool CheckBoxDelegate::editorEvent(QEvent *event,
	QAbstractItemModel *model,
	const QStyleOptionViewItem &option,
	const QModelIndex &index) {

	if (_readOnly)
	{
		return false;
	}

	if (index.column() == column) {
		if ((event->type() == QEvent::MouseButtonRelease)) {
			if (event->type() == QEvent::MouseButtonDblClick) {
				return true;
			}

			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() == Qt::LeftButton &&
				CheckBoxRect(option).contains(mouseEvent->pos())) {

				bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
				int data = checked ? 0 : 1;     //互逆
				emit setChecked(index, !checked);
				return model->setData(index, data, Qt::EditRole);
			}
			else
			{
				return false;
			}
		}
		else if (event->type() == QEvent::KeyPress) {
			if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space &&
				static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
				return false;
			}
			else
			{
				return true;
			}
		}
		return false;
	}
	else {
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}
}