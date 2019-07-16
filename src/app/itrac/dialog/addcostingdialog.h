#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

class QDateEdit;
class QStandardItem;
class QItemSelectionModel;
class QStandardItemModel;
class TableView;

class Addcostingdialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	Addcostingdialog(QWidget *parent = Q_NULLPTR);
	~Addcostingdialog();

protected:
	void accept() override;

private slots:
	void tableItemChanged(QStandardItem *item);

private:
	void add();
	void remove();
	void dateChanged(const QDate &date);

	void initCostView();
	void reSumTotal();
	void savePriceTemplate();

	QDateEdit *_dateEdit;
	TableView *_view;
	QItemSelectionModel *_theSelectionModel;
	QStandardItemModel *_model;
	QVariantMap *_currentMonthPacktypeCountMap = Q_NULLPTR;
	QVariantMap *_priceMap = Q_NULLPTR;;
};
