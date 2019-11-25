#pragma once

#include "labelprinter_global.h"
#include "qstring.h"
#include "qzint.h"

struct PackageLabel
{
	QString packageId;			
	QString packageName;		
	QString packageFrom;		
	QString packageType;		
	QString disinDate;			
	QString expiryDate;			
	QString operatorName;		
	QString assessorName;		
	int count = 0;				
};

struct ClinicLabel				
{
	QString packageId;			
	QString packageName;		
	QString packageFrom;		
	QString disinDate;			
	QString expiryDate;			
	QString operatorName;		
	QString assessorName;		
};

struct SterilizeLabel			
{
	QString sterilizeId;		
	QString sterilizeName;		
	QString sterilizeDate;		
	QString sterilizeTime;		
	int panNum = 0;				
	int packageNum = 0;			
};

struct package
{
	QString name;
	int count = 0;
};

enum PaperType
{
	recycle,dispatch
};

struct Issues
{
	QString orderId;
	QString deptName;
	QString date;
	QString operName;
	QString applyName;
	QList<package> packages;
};

class LABELPRINTER_EXPORT LabelPrinter
{
public:
	int open(const QString &strPrinterName);
	void close();
	QString getName() const;

	int printPackageLabel(const PackageLabel &label);

	int printClinicLabel(const ClinicLabel &label);

	int printSterilizedLabel(const SterilizeLabel &label);

	virtual void setFontName(const QString &strFontName) = 0;

	virtual void setInversion(const bool inver) = 0;

	virtual void setBold(const bool isBold) = 0;

	int printIssue(const Issues &issues, PaperType type);
private:
	QString m_szPrinterName;
	
	virtual int buildPackageLabels(const PackageLabel &label, QString &szLabelBuilder) = 0;
	virtual int buildClinicLabels(const ClinicLabel &label, QString &szLabelBuilder) = 0;
	virtual int buildSterilizedLabels(const SterilizeLabel &label, QString &szLabelBuilder) = 0;

	//virtual printer, draw jpg to c:/cssd
	int printPackageLabelsToImg(const PackageLabel &label);
	int printClinicLabelsToImg(const ClinicLabel &label);
	int printSterilizedLabelsToImg(const SterilizeLabel &label);

	int printPage(const Issues &issues, PaperType type, int page, int total);
	
	mutable Zint::QZint bc;
	int _count;
};

class LABELPRINTER_EXPORT PrinterFactory {
public:
	enum PRINTER_TYPE {
		ZEBRA_GT8,
		//IMG_PRINTER,
	};

	LabelPrinter * Create(PRINTER_TYPE type);
};
