#pragma once

#include "cssd_overlay.h"

class DeptEdit;
class DispatchPackageView;
class OrDispatchPanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	OrDispatchPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();

private:
	void reset();

	DeptEdit * _deptEdit;
	DispatchPackageView * _pkgView;
};
