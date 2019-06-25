#pragma once

#include "cssd_overlay.h"

class PackPlateView;
class PackPanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	PackPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
	//void print(const std::list<std::list<std::string>> & labels);

private:
	PackPlateView * _plateView;
};
