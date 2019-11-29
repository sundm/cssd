#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"

class DeviceArea;
class PlateView;
struct Package;

class WashPanel : public CssdOverlayPanel, public JsonHttpClient
{
	Q_OBJECT

public:
	WashPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();

	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);

private:
	void reset();

	DeviceArea * _deviceArea;
	PlateView * _plateView;

	QList<Package> _pkgList;
};
