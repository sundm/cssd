#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

class TableView;
class QStandardItemModel;
class WaitingSpinner;
class RePrintDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	RePrintDialog(QWidget *parent);
	~RePrintDialog();

protected:
	void accept() override;

private:
	void loadSource();

	TableView * _view;
	QStandardItemModel * _model;

	WaitingSpinner *_waiter;
};
