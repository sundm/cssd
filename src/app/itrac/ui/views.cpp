#include "views.h"
#include <QHeaderView>

TableView::TableView(QWidget *parent)
	: QTableView(parent)
{
	setSelectionBehavior(QAbstractItemView::SelectRows);

	QHeaderView *header = horizontalHeader();
	header->setStretchLastSection(true);
	header->setSectionResizeMode(QHeaderView::Stretch);
}

int TableView::findRow(int column, int role, const QVariant &value) {
	if (QAbstractItemModel *model = this->model()) {
		QModelIndexList matches = model->match(model->index(0, column), role, value, 1);
		return matches.isEmpty() ? -1 : matches[0].row();
	}
	return -1;
}

QModelIndexList TableView::selectedRows() const {
	return selectionModel()->selectedRows();
}

void TableView::clear() {
	if (QAbstractItemModel *model = this->model()) {
		model->removeRows(0, model->rowCount());
	}
}

void TableView::removeSeletedRows() {
	if (QAbstractItemModel *model = this->model()) {
		QItemSelectionModel *selModel = selectionModel();
		QModelIndexList indexes = selModel->selectedRows();
		for (int i = indexes.count(); i > 0; i--)
			model->removeRow(indexes.at(i - 1).row());
	}
}
