#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

class TableView;
class QStandardItemModel;
class WaitingSpinner;
class ImportExtDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	ImportExtDialog(QWidget *parent);
	~ImportExtDialog();

protected:
	void accept() override;

signals:
	void extPkgImport(const QString&, const QString&, const QString&);
private slots:
	void onRowClicked(const QModelIndex &);

private:
	void loadExt();

	TableView * _view;
	QStandardItemModel * _model;

	TableView * _detailView;
	QStandardItemModel * _detailModel;
	WaitingSpinner *_waiter;
};
