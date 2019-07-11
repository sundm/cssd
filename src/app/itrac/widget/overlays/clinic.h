#pragma once

#include "cssd_overlay.h"

namespace Composite {
	class Title;
}

class TableView;
class QStandardItemModel;

class ClinicPanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	ClinicPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;
	void reset();

private slots:
	void commit();
	void loadOrders();
	void showDetail(const QModelIndex &);

private:
	TableView *_view;
	TableView *_detailView;
	QStandardItemModel *_model;
	QStandardItemModel *_detailModel;
	Composite::Title *_title;

	void updatePlate(const QString &);
};
