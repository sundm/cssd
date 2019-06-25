#pragma once
#include "labelprinter.h"

class ZebraGT8 :
	public LabelPrinter
{
public:
	ZebraGT8();
	~ZebraGT8();
	void setFontName(const QString &strFontName);
	void setInversion(const bool inver);
	void setBold(const bool isBold);
private:
	QString m_szFontName;
	bool m_isInver;
	bool m_isBold;
	bool m_isInit;

	int readLabelTemple();
	QString m_szPackageLabelTemple;
	QString m_szClinicLabelTemple;
	QString m_szSterilizeLabelTemple;


	int buildPackageLabels(const PackageLabel &label, QString &szLabelBuilder);
	int buildClinicLabels(const ClinicLabel &label, QString &szLabelBuilder);
	int buildSterilizedLabels(const SterilizeLabel &label, QString &szLabelBuilder);

	int converToImg(const QString &sourceStr, const QString &imgName, QString &imgStr);
};

