#pragma once

#include "labelprinter_global.h"
#include "qstring.h"

struct PackageLabel					//�����ұ�ǩ
{
	QString packageId;				//��ID
	QString packageName;			//����
	QString packageFrom;			//����Դ
	QString packageType;			//�������
	QString disinDate;				//�������
	QString expiryDate;				//ʧЧ����
	QString operatorName;			//����Ա
	QString assessorName;			//���Ա
	int count = 0;					//����
};

struct ClinicLabel					//�ٴ���ǩ
{
	QString packageId;				//��ID
	QString packageName;			//����
	QString packageFrom;			//����Դ
	QString disinDate;				//�������
	QString expiryDate;				//ʧЧ����
	QString operatorName;			//����Ա
	QString assessorName;			//���Ա
};

struct SterilizeLabel				//������ǩ
{
	QString sterilizeId;			//�����ID
	QString sterilizeName;			//�������
	QString sterilizeDate;			//�������
	QString sterilizeTime;			//���ʱ��
	int panNum = 0;					//�������
	int packageNum = 0;				//�������
};

class LABELPRINTER_EXPORT LabelPrinter
{
public:
	int open(const QString &strPrinterName);
	void close();
	QString getName() const;

	//��ӡ�����Ұ���ǩ
	int printPackageLabel(const PackageLabel &label);
	//��ӡ�ٴ�����ǩ
	int printClinicLabel(const ClinicLabel &label);
	//��ӡ������ǩ
	int printSterilizedLabel(const SterilizeLabel &label);
	//���ô�ӡ���壬Ĭ��Ϊ����
	virtual void setFontName(const QString &strFontName) = 0;
	//���ô�ӡ����Ĭ��Ϊ����
	virtual void setInversion(const bool inver) = 0;
	//���ô�ӡ�����Ƿ�Ϊ���壬Ĭ��Ϊ��
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
	};

	LabelPrinter * Create(PRINTER_TYPE type);
};
