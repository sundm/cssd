#include "labelprinter.h"
#include <tchar.h>
#include <windows.h>
#include <winspool.h>
#include <QFile>
#include <QMutex>
#include <QDateTime>
#include <qtextstream.h>

#include "ZebraGT8.h"
#include "qdebug.h"

HANDLE m_hPrinter;
BOOL RawDataToPrinter(const QString &labelContent);
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

LabelPrinter* PrinterFactory::Create(PRINTER_TYPE type)
{
	switch (type)
	{
	case ZEBRA_GT8:
		return new ZebraGT8();
	default:
		break;
	}
	return NULL;
}

int LabelPrinter::open(const QString & strPrinterName)
{
	qInstallMessageHandler(myMessageOutput);

	m_szPrinterName = strPrinterName;

	m_hPrinter = NULL;
	if (!OpenPrinter((LPTSTR)m_szPrinterName.utf16(), &m_hPrinter, NULL))
	{
		qDebug() << "open " << m_szPrinterName << " Error!";
		return PRINTER_OPEN_ERR;
	}

	return OK;
}

void LabelPrinter::close()
{
	if (m_hPrinter == NULL) return;

	ClosePrinter(m_hPrinter);
}

QString LabelPrinter::getName() const
{
	return m_szPrinterName;
}

int LabelPrinter::printPackageLabel(const PackageLabel & label)
{
	QString szLabelContentBuilder;
	buildPackageLabels(label, szLabelContentBuilder);
	if (szLabelContentBuilder.isEmpty()) {
		qDebug() << "package label string builder is empty!";
		return INVALID_ERR;
	}
	return RawDataToPrinter(szLabelContentBuilder);
}

int LabelPrinter::printClinicLabel(const ClinicLabel & label)
{
	QString szLabelContentBuilder;
	buildClinicLabels(label, szLabelContentBuilder);
	if (szLabelContentBuilder.isEmpty()) {
		qDebug() << "package label string builder is empty!";
		return INVALID_ERR;
	}
	return RawDataToPrinter(szLabelContentBuilder);
}

int LabelPrinter::printSterilizedLabel(const SterilizeLabel & label)
{
	QString szLabelContentBuilder;
	buildSterilizedLabels(label, szLabelContentBuilder);
	if (szLabelContentBuilder.isEmpty()) {
		qDebug() << "package label string builder is empty!";
		return INVALID_ERR;
	}
	return RawDataToPrinter(szLabelContentBuilder);
}



BOOL RawDataToPrinter(const QString &labelContent)
{
	DOC_INFO_1 DocInfo;
	DWORD      dwJob;
	DWORD      dwBytesWritten;
	QString m_t = labelContent;
	QByteArray ba = m_t.toLocal8Bit();
	LPBYTE lpData = (LPBYTE)ba.data();
	DWORD dwCount = strlen(ba.data());

	DocInfo.pDocName = TEXT("My Document");
	DocInfo.pOutputFile = NULL;
	DocInfo.pDatatype = TEXT("RAW");

	if ((dwJob = StartDocPrinter(m_hPrinter, 1, (LPBYTE)&DocInfo)) == 0)
	{
		ClosePrinter(m_hPrinter);
		return FALSE;
	}

	if (!StartPagePrinter(m_hPrinter))
	{
		EndDocPrinter(m_hPrinter);
		ClosePrinter(m_hPrinter);
		return FALSE;
	}
	
	if (!WritePrinter(m_hPrinter, lpData, dwCount, &dwBytesWritten))
	{

		EndPagePrinter(m_hPrinter);
		EndDocPrinter(m_hPrinter);
		ClosePrinter(m_hPrinter);
		return FALSE;
	}
	 
	if (!EndPagePrinter(m_hPrinter))
	{

		EndDocPrinter(m_hPrinter);
		ClosePrinter(m_hPrinter);
		return FALSE;
	}
	 
	if (!EndDocPrinter(m_hPrinter))
	{

		ClosePrinter(m_hPrinter);
		return FALSE;
	}
	
	//ClosePrinter(m_hPrinter);
	
	if (dwBytesWritten != dwCount)
	{

		return FALSE;
	}
	return TRUE;
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	static QMutex mutex;
	mutex.lock();

	QByteArray localMsg = msg.toLocal8Bit();

	QString strMsg("");
	switch (type)
	{
	case QtDebugMsg:
		strMsg = QString("Debug:");
		break;
	case QtWarningMsg:
		strMsg = QString("Warning:");
		break;
	case QtCriticalMsg:
		strMsg = QString("Critical:");
		break;
	case QtFatalMsg:
		strMsg = QString("Fatal:");
		break;
	}

	QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
	QString strMessage = QString("Message:%1 File:%2  Line:%3  Function:%4  DateTime:%5")
		.arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function).arg(strDateTime);

	QFile file("log.txt");
	file.open(QIODevice::ReadWrite | QIODevice::Append);
	QTextStream stream(&file);
	stream << strMessage << "\r\n";
	file.flush();
	file.close();

	mutex.unlock();
}
