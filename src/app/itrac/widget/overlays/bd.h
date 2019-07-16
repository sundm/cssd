#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"

class DeviceArea;
class BDPanel : public CssdOverlayPanel, public JsonHttpClient
{
	Q_OBJECT

public:
	BDPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commitQualified();
	void commitUnqualified();

private:
	void commit(bool);
	DeviceArea * _deviceArea;
};
