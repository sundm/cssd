#pragma once

#include "cssd_overlay.h"

class PackPlateView;
class PackageDetailView;

class PackPanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	PackPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
	void addPlate();
	void showDetail(const QModelIndex &index);
	void reprint();
	void abnormal();
	void updateRecord(int pkg_record);
private:
	PackPlateView * _plateView;
	PackageDetailView * _detailView;

	int _row;
};
