#pragma once

#include "labelprinter_global.h"
#include "qstring.h"

struct PackageLabel					//手术室标签
{
	QString packageId;				//包ID
	QString packageName;			//包名
	QString packageFrom;			//包来源
	QString packageType;			//打包类型
	QString disinDate;				//灭菌日期
	QString expiryDate;				//失效日期
	QString operatorName;			//操作员
	QString assessorName;			//审核员
	int count = 0;					//数量
};

struct ClinicLabel					//临床标签
{
	QString packageId;				//包ID
	QString packageName;			//包名
	QString packageFrom;			//包来源
	QString disinDate;				//灭菌日期
	QString expiryDate;				//失效日期
	QString operatorName;			//操作员
	QString assessorName;			//审核员
};

struct SterilizeLabel				//消毒标签
{
	QString sterilizeId;			//灭菌器ID
	QString sterilizeName;			//灭菌器名
	QString sterilizeDate;			//灭菌日期
	QString sterilizeTime;			//灭菌时间
	int panNum = 0;					//灭菌锅次
	int packageNum = 0;				//灭菌包数
};

class LABELPRINTER_EXPORT LabelPrinter
{
public:
	int open(const QString &strPrinterName);
	void close();
	QString getName() const;

	//打印手术室包标签
	int printPackageLabel(const PackageLabel &label);
	//打印临床包标签
	int printClinicLabel(const ClinicLabel &label);
	//打印消毒标签
	int printSterilizedLabel(const SterilizeLabel &label);
	//设置打印字体，默认为黑体
	virtual void setFontName(const QString &strFontName) = 0;
	//设置打印方向，默认为反向
	virtual void setInversion(const bool inver) = 0;
	//设置打印字体是否为粗体，默认为否
	virtual void setBold(const bool isBold) = 0;

private:
	QString m_szPrinterName;

	virtual int buildPackageLabels(const PackageLabel &label, QString &szLabelBuilder) = 0;
	virtual int buildClinicLabels(const ClinicLabel &label, QString &szLabelBuilder) = 0;
	virtual int buildSterilizedLabels(const SterilizeLabel &label, QString &szLabelBuilder) = 0;


};

class LABELPRINTER_EXPORT PrinterFactory {
public:
	enum PRINTER_TYPE {
		ZEBRA_GT8,
		PNG
	};

	LabelPrinter * Create(PRINTER_TYPE type);
};
