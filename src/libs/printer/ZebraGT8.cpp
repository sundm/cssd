#include "ZebraGT8.h"
#include <Windows.h>
#include <string>
#include <QFile>
#include <qtextstream.h>
#include "qdebug.h"

HINSTANCE dll_instance;
typedef int(__stdcall *GETFONTHEX)(LPSTR, LPSTR, LPSTR, int, int, int, int, int, LPSTR);
int font_orient = 0;
int font_hight = 30;

GETFONTHEX pDLLAddress;
TCHAR *CharToWchar(const QString &str);

ZebraGT8::ZebraGT8()
{
	m_isInit = true;
	//dll_instance = LoadLibrary(CharToWchar(QString("fnthex32.dll")));
    dll_instance = LoadLibrary(L"d:\\Project\\cssd\\repo\\v12\\bin\\x64-Debug\\Fnthex32.DLL");
    DWORD res = GetLastError();
	pDLLAddress = (GETFONTHEX)(GetProcAddress(dll_instance, "GETFONTHEX"));

	if (dll_instance == NULL || pDLLAddress == NULL) {
		qDebug() << "load fnthex32.dll error!";
		m_isInit = FALSE;
		return;
	}

	m_szFontName = QString::fromLocal8Bit("黑体");
	m_isBold = false;
	m_isInver = false;

	if (OK != readLabelTemple()) {
		qDebug() << "read labeltemple file error!";
		m_isInit = FALSE;
		return;
	}
}


ZebraGT8::~ZebraGT8()
{
}

int ZebraGT8::buildPackageLabels(const PackageLabel & label, QString & szLabelBuilder)
{
	if (!m_isInit) return INIT_ENV_ERR;

	QString imgPackageName;
	converToImg(label.packageName, QString("temp1"), imgPackageName);

	QString strPackageFrom = QString("%1[%2]").arg(label.packageFrom).arg(label.packageType);
	QString imgPackageFrom;
	converToImg(strPackageFrom, QString("temp2"), imgPackageFrom);

	QString strDisinDate = QString::fromLocal8Bit("灭菌日期:%1").arg(label.disinDate);
	QString imgDisinDate;
	converToImg(strDisinDate, QString("temp3"), imgDisinDate);

	QString strExpiryDate = QString::fromLocal8Bit("失效日期:%1").arg(label.expiryDate);
	QString imgExpiryDate;
	converToImg(strExpiryDate, QString("temp4"), imgExpiryDate);

	QString strCount = QString::fromLocal8Bit("回收件数:%1").arg(label.count);
	QString imgCount;
	converToImg(strCount, QString("temp5"), imgCount);

	QString strOperName = QString::fromLocal8Bit("配包人:%1").arg(label.operatorName);
	QString imgOperName;
	converToImg(strOperName, QString("temp6"), imgOperName);

	QString strAss = QString::fromLocal8Bit("审核人:%1").arg(label.assessorName);
	QString imgAss;
	converToImg(strAss, QString("temp7"), imgAss);
	
	font_hight = 28;
	bool isBd = m_isBold;
	m_isBold = true;
	QString leftCode = label.packageId.left(label.packageId.length() / 2);
	QString rightCode = label.packageId.right(label.packageId.length() / 2);
	QString imgleftCode;
	converToImg(leftCode, QString("temp8"), imgleftCode);
	QString imgrightCode;
	converToImg(rightCode, QString("temp9"), imgrightCode);

	font_hight = 30;
	m_isBold = isBd;

	QString szInver = QString::fromLocal8Bit("N");
	if (m_isInver) szInver = QString::fromLocal8Bit("I");

	szLabelBuilder = QString(m_szPackageLabelTemple).arg(label.packageId).arg(label.packageId)
		.arg(imgPackageName).arg(imgPackageFrom)
		.arg(imgDisinDate).arg(imgExpiryDate).arg(imgCount)
		.arg(imgOperName).arg(imgAss).arg(imgleftCode).arg(imgrightCode)
		.arg(szInver);
	return OK;
}

int ZebraGT8::buildClinicLabels(const ClinicLabel & label, QString & szLabelBuilder)
{
	if (!m_isInit) return INIT_ENV_ERR;

	QString strPackageName = QString::fromLocal8Bit("物品:%1").arg(label.packageName);
	QString imgPackageName;
	converToImg(strPackageName, QString("temp1"), imgPackageName);

	QString strPackageFrom = QString::fromLocal8Bit("科室:%1").arg(label.packageFrom);
	QString imgPackageFrom;
	converToImg(strPackageFrom, QString("temp2"), imgPackageFrom);

	QString strDisinDate = QString::fromLocal8Bit("灭菌日期:%1").arg(label.disinDate);
	QString imgDisinDate;
	converToImg(strDisinDate, QString("temp3"), imgDisinDate);

	QString strExpiryDate = QString::fromLocal8Bit("失效日期:%1").arg(label.expiryDate);
	QString imgExpiryDate;
	converToImg(strExpiryDate, QString("temp4"), imgExpiryDate);

	QString strOperName = QString::fromLocal8Bit("配包人:%1").arg(label.operatorName);
	QString imgOperName;
	converToImg(strOperName, QString("temp5"), imgOperName);

	QString strAss = QString::fromLocal8Bit("审核人:%1").arg(label.assessorName);
	QString imgAss;
	converToImg(strAss, QString("temp6"), imgAss);

	font_hight = 28;
	bool isBd = m_isBold;
	m_isBold = true;

	QString imgCode;
	converToImg(label.packageId, QString("temp7"), imgCode);

	font_hight = 30;
	m_isBold = isBd;

	QString szInver = QString::fromLocal8Bit("N");
	if (m_isInver) szInver = QString::fromLocal8Bit("I");

	szLabelBuilder = QString(m_szClinicLabelTemple).arg(label.packageId)
		.arg(imgPackageName).arg(imgPackageFrom)
		.arg(imgDisinDate).arg(imgExpiryDate)
		.arg(imgOperName).arg(imgAss).arg(imgCode)
		.arg(szInver);
	return OK;
}

int ZebraGT8::buildSterilizedLabels(const SterilizeLabel & label, QString & szLabelBuilder)
{
	if (!m_isInit) return INIT_ENV_ERR;

	QString imgSterilizeNam;
	converToImg(label.sterilizeName, QString("temp1"), imgSterilizeNam);

	QString strDisinDate = QString::fromLocal8Bit("灭菌日期:%1").arg(label.sterilizeDate);
	QString imgDisinDate;
	converToImg(strDisinDate, QString("temp2"), imgDisinDate);

	QString strDisinTime = QString::fromLocal8Bit("灭菌时间:%1").arg(label.sterilizeTime);
	QString imgDisinTime;
	converToImg(strDisinTime, QString("temp3"), imgDisinTime);

	QString strPanNum = QString::fromLocal8Bit("第%1锅次").arg(label.panNum);
	QString imgPanNum;
	converToImg(strPanNum, QString("temp4"), imgPanNum);

	QString strPackageNum = QString::fromLocal8Bit("共%1包").arg(label.packageNum);
	QString imgPackageNum;
	converToImg(strPackageNum, QString("temp5"), imgPackageNum);

	font_hight = 28;
	bool isBd = m_isBold;
	m_isBold = true;

	QString imgSterilizedId;
	converToImg(label.sterilizeId, QString("temp6"), imgSterilizedId);

	font_hight = 30;
	m_isBold = isBd;



	QString szInver = QString::fromLocal8Bit("N");
	if (m_isInver) szInver = QString::fromLocal8Bit("I");

	szLabelBuilder = QString(m_szSterilizeLabelTemple).arg(label.sterilizeId)
		.arg(imgSterilizeNam)
		.arg(imgDisinDate).arg(imgDisinTime)
		.arg(imgPanNum).arg(imgPackageNum).arg(imgSterilizedId)
		.arg(szInver);

	return OK;
}

int ZebraGT8::converToImg(const QString &sourceStr, const QString &imgName, QString &imgStr)
{
	char chBufferChineseZPL[3096] = "\0";
	int res = 0;

	std::string tempSource = sourceStr.toLocal8Bit();
	std::string fontName = m_szFontName.toLocal8Bit();
	std::string tempName = imgName.toLocal8Bit();

	res = pDLLAddress(const_cast<char *>(tempSource.c_str()), const_cast<char *>(fontName.c_str()), const_cast<char *>(tempName.c_str()),
		font_orient, font_hight, 0, m_isBold, 0, chBufferChineseZPL);
	if (res == 0) {
		qDebug() << "conver string to image error!";
		return PRINTER_CONVER_ERR;
	}
	
	imgStr = QString::fromUtf8(chBufferChineseZPL, res);

	return OK;
}

void ZebraGT8::setFontName(const QString & strFontName)
{
	m_szFontName = strFontName;
}

void ZebraGT8::setInversion(const bool inver)
{
	m_isInver = inver;
}

void ZebraGT8::setBold(const bool isBold)
{
	m_isBold = isBold;
}

int ZebraGT8::readLabelTemple()
{
	QFile templeFile("label/labelTemple");
	if (!templeFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Can't Open labelTemple File";
		return OPEN_TEMPLE_ERR;
	}
	QTextStream out(&templeFile);
	m_szPackageLabelTemple = out.readLine();
	m_szClinicLabelTemple = out.readLine();
	m_szSterilizeLabelTemple = out.readLine();

	templeFile.close();
	return OK;
}

TCHAR *CharToWchar(const QString &str)
{
	QByteArray ba = str.toUtf8();
	char *data = ba.data();
	int charLen = strlen(data);
	int len = MultiByteToWideChar(CP_ACP, 0, data, charLen, NULL, 0);
	TCHAR *buf = new TCHAR[len + 1];
	MultiByteToWideChar(CP_ACP, 0, data, charLen, buf, len);
	buf[len] = '\0';
	return buf;
}


