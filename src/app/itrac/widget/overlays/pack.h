#pragma once

#include "cssd_overlay.h"

class DeptEdit;
class PackageEdit;
class QSpinBox;
class TableView;
class XPicture;
class UnusualInstrumentView;
class PackageInfoView;
class PackageDetailView;
class QStandardItemModel;

class PackPanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	PackPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();

	void reprint();
	void abnormal();

	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);
private:
	void reset();

	PackageInfoView * _pkgView;
	PackageDetailView * _detailView;
	UnusualInstrumentView * _unusualView;

	QHash<QString, QString> * _codeMap;
	QStringList * _scannedCodes;
	QStringList * _unusualCodes;

	XPicture * _pkgImg;
	XPicture * _insImg;
	int _row;
	int _step;
};
