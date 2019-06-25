#pragma once

#include <QDialog>

class TableView;
class QStandardItemModel;
class ImportExtDialog : public QDialog
{
	Q_OBJECT

public:
	ImportExtDialog(QWidget *parent);
	~ImportExtDialog();

protected:
	void accept() override;

private:
	void loadExt();

	TableView * _view;
	QStandardItemModel * _model;
};
