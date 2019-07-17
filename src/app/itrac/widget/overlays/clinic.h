#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"

namespace Composite {
	class Title;
}

class TableView;
class QStandardItemModel;
class WaitingSpinner;

class ClinicPanel : public CssdOverlayPanel, public JsonHttpClient
{
	Q_OBJECT

public:
	ClinicPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;
	void reset();

private slots:
	void commit();
	void addPlate();
	void loadOrders();
	void showDetail(const QModelIndex &);

private:
	TableView *_view;
	TableView *_detailView;
	QStandardItemModel *_model;
	QStandardItemModel *_detailModel;
	Composite::Title *_title;
	WaitingSpinner *_waiter;

	void updatePlate(const QString &);
};
