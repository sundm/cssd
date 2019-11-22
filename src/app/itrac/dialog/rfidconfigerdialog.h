#pragma once

#include <QDialog>

class QComboBox;
class TableView;
class QStandardItemModel;

class ConfigRfidDialog : public QDialog
{
	Q_OBJECT

public:
	ConfigRfidDialog(QWidget *parent = Q_NULLPTR);

protected:
	void accept() override;

private slots:
	void addEntry();
	void removeEntry();

private:
	void initView();

	QComboBox *_comBox;
	TableView *_view;
	QStandardItemModel *_model;
};
