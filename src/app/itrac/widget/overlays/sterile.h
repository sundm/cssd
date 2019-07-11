#pragma once

#include "cssd_overlay.h"

class DeviceArea;
class SterilePackageView;
class SterilePanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	SterilePanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();

private:
	DeviceArea * _deviceArea;
	SterilePackageView * _pkgView;
	void reset();
};
