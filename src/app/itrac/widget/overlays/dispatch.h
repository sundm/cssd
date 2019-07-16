#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"

class DeptEdit;
class DispatchPackageView;
class OrDispatchPanel : public CssdOverlayPanel, public JsonHttpClient
{
	Q_OBJECT

public:
	OrDispatchPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
	void addEntry();
	void removeEntry();
private:
	void reset();

	DeptEdit * _deptEdit;
	DispatchPackageView * _pkgView;
};
