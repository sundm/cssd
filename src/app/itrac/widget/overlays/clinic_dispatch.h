#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"

namespace Composite {
	class Title;
}

namespace Ui {
	class PrimaryButton;
}

class TableView;
class QStandardItemModel;
class QStringList;
class WaitingSpinner;

class ClinicDispatchPanel : public CssdOverlayPanel, public JsonHttpClient
{
	Q_OBJECT

public:
	ClinicDispatchPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;
	void reset();

private slots:
	void commit();
	void loadOrders();
	void showDetail(const QModelIndex &);
	bool hasPackage(const QString& id);
	void addPackage(const QString& id);
	void addEntry();
	void removeEntry();
private:
	bool checkNumber();

	TableView *_view;
	TableView *_detailView;
	QStandardItemModel *_model;
	QStandardItemModel *_detailModel;
	Composite::Title *_title;

	Composite::Title *_scan_title;
	TableView *_scanView;
	QStandardItemModel *_scanModel;

	QStringList  *_pktList;
	QStringList  *_codeList;
	QMap<QString, int> *_detailMap;
	QMap<QString, int> *_scanMap;

	Ui::PrimaryButton *_commitButton;
	WaitingSpinner *_waiter;
};