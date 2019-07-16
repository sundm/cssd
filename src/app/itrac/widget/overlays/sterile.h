#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"

class DeviceArea;
class SterilePackageView;
class SterilePanel : public CssdOverlayPanel, public JsonHttpClient
{
	Q_OBJECT

public:
	SterilePanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
	void addEntry();
	void removeEntry();

private:
	DeviceArea * _deviceArea;
	SterilePackageView * _pkgView;
	void reset();
};
