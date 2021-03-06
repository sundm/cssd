#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"

class DeviceArea;
class PlateView;
class WashPanel : public CssdOverlayPanel, public JsonHttpClient
{
	Q_OBJECT

public:
	WashPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
	void addPlate();
private:
	void reset();

	DeviceArea * _deviceArea;
	PlateView * _plateView;
};
