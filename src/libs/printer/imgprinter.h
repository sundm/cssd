#pragma once

#include "labelprinter.h"
#include "qzint.h"

class ImgPrinter : public LabelPrinter
{

public:
	ImgPrinter();
	~ImgPrinter();

	void setFontName(const QString &strFontName);
	void setInversion(const bool inver);
	void setBold(const bool isBold);

private:
	int buildPackageLabels(const PackageLabel &label, QString &szLabelBuilder);
	int buildClinicLabels(const ClinicLabel &label, QString &szLabelBuilder);
	int buildSterilizedLabels(const SterilizeLabel &label, QString &szLabelBuilder);

	mutable Zint::QZint bc;
};
