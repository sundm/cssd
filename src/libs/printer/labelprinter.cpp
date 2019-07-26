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
int g_type;

LabelPrinter* PrinterFactory::Create(PRINTER_TYPE type)
{
	g_type = type;
	switch (type)
	{
	case ZEBRA_GT8:
		return new ZebraGT8();
	//case IMG_PRINTER:
	//	return new ImgPrinter();
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
#ifdef SELL_DEMO
	return OK;	//if (g_type == PrinterFactory::IMG_PRINTER) return OK;
#else
	if (!OpenPrinter((LPTSTR)m_szPrinterName.utf16(), &m_hPrinter, NULL))
	{
		qDebug() << "open " << m_szPrinterName << " Error!";
		return PRINTER_OPEN_ERR;
	}

	return OK;
#endif //SELL_DEMO

	
	
	
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

	//if (g_type == PrinterFactory::IMG_PRINTER) return buildPackageLabels(label, szLabelContentBuilder);
#ifdef SELL_DEMO
	return printPackageLabelsToImg(label);
#else
	buildPackageLabels(label, szLabelContentBuilder);
	if (szLabelContentBuilder.isEmpty()) {
		qDebug() << "package label string builder is empty!";
		return INVALID_ERR;
	}

	return RawDataToPrinter(szLabelContentBuilder);
#endif // SELL_DEBUG


}

int LabelPrinter::printClinicLabel(const ClinicLabel & label)
{
	QString szLabelContentBuilder;

	//if (g_type == PrinterFactory::IMG_PRINTER) return buildClinicLabels(label, szLabelContentBuilder);

#ifdef SELL_DEMO
	return printClinicLabelsToImg(label);
#else
	buildClinicLabels(label, szLabelContentBuilder);
	if (szLabelContentBuilder.isEmpty()) {
		qDebug() << "package label string builder is empty!";
		return INVALID_ERR;
	}
	return RawDataToPrinter(szLabelContentBuilder);
#endif // SELL_DEBUG

	
}

int LabelPrinter::printSterilizedLabel(const SterilizeLabel & label)
{
	QString szLabelContentBuilder;

	//if (g_type == PrinterFactory::IMG_PRINTER) return buildSterilizedLabels(label, szLabelContentBuilder);

#ifdef SELL_DEMO
	return printSterilizedLabelsToImg(label);
#else
	buildSterilizedLabels(label, szLabelContentBuilder);
	if (szLabelContentBuilder.isEmpty()) {
		qDebug() << "package label string builder is empty!";
		return INVALID_ERR;
	}
	return RawDataToPrinter(szLabelContentBuilder);
#endif // SELL_DEBUG

}

int LabelPrinter::printPackageLabelsToImg(const PackageLabel &label) {
	QPixmap *pix = new QPixmap(780, 600);
	QPainter *painter = new QPainter(pix);
	pix->fill(Qt::transparent);

	painter->begin(pix);

	//draw background
	QPixmap *backgroundPix = new QPixmap(780, 600);
	backgroundPix->load("c:/cssd/template/background_h.jpg");
	painter->drawPixmap(0, 0, *backgroundPix);

	//draw text
	QFont titleFont("黑体", 24, QFont::Bold);
	painter->setFont(titleFont);
	painter->drawText(180, 70, label.packageName);
	QFont txtFont("黑体", 18, QFont::Bold);
	painter->setFont(txtFont);
	QString packageTpye = QString("%1[%2]").arg(label.packageFrom).arg(label.packageType);
	painter->drawText(180, 120, packageTpye);
	painter->drawText(180, 170, QString::fromLocal8Bit("灭菌日期:%1").arg(label.disinDate));
	painter->drawText(180, 220, QString::fromLocal8Bit("失效日期:%1").arg(label.expiryDate));
	painter->drawText(180, 270, QString::fromLocal8Bit("回收件数:%1").arg(QString::number(label.count)));
	painter->drawText(180, 320, QString::fromLocal8Bit("配包人:%1").arg(label.operatorName));
	painter->drawText(500, 320, QString::fromLocal8Bit("审核人:%1").arg(label.assessorName));

	//draw barcode
	bc.setText(label.packageId);
	bc.setBorderType(Zint::QZint::NO_BORDER);
	bc.setHideText(true);

	bc.render(*painter, QRectF(150, 360, 500, 200), Zint::QZint::IgnoreAspectRatio);
	painter->drawText(240, 570, label.packageId);

	painter->end();

	//save file
	QString filePath = QString("C:/cssd/%1.jpg").arg(label.packageId);
	QFile file(filePath);
	file.open(QIODevice::WriteOnly);
	pix->save(&file, "JPG");
	return OK;
}

int LabelPrinter::printClinicLabelsToImg(const ClinicLabel &label) {
	QPixmap *pix = new QPixmap(780, 430);
	QPainter *painter = new QPainter(pix);
	pix->fill(Qt::transparent);

	painter->begin(pix);

	//draw background
	QPixmap *backgroundPix = new QPixmap(780, 430);
	backgroundPix->load("c:/cssd/template/background_w.jpg");
	painter->drawPixmap(0, 0, *backgroundPix);

	//draw text
	QFont titleFont("黑体", 24, QFont::Bold);
	painter->setFont(titleFont);
	painter->drawText(180, 70, label.packageName);
	QFont txtFont("黑体", 18, QFont::Bold);
	painter->setFont(txtFont);
	painter->drawText(180, 120, QString::fromLocal8Bit("科室:%1").arg(label.packageFrom));
	painter->drawText(180, 160, QString::fromLocal8Bit("灭菌日期:%1").arg(label.disinDate));
	painter->drawText(180, 200, QString::fromLocal8Bit("失效日期:%1").arg(label.expiryDate));
	painter->drawText(180, 240, QString::fromLocal8Bit("配包人:%1").arg(label.operatorName));
	painter->drawText(500, 240, QString::fromLocal8Bit("审核人:%1").arg(label.assessorName));

	//draw barcode
	bc.setText(label.packageId);
	bc.setBorderType(Zint::QZint::NO_BORDER);
	bc.setHideText(true);

	bc.render(*painter, QRectF(180, 260, 400, 150), Zint::QZint::IgnoreAspectRatio);
	painter->drawText(240, 420, label.packageId);

	painter->end();

	//save file
	QString filePath = QString("C:/cssd/%1.jpg").arg(label.packageId);
	QFile file(filePath);
	file.open(QIODevice::WriteOnly);
	pix->save(&file, "JPG");

	return OK;
}

int LabelPrinter::printSterilizedLabelsToImg(const SterilizeLabel &label) {
	QPixmap *pix = new QPixmap(780, 430);
	QPainter *painter = new QPainter(pix);
	pix->fill(Qt::transparent);

	painter->begin(pix);

	//draw background
	QPixmap *backgroundPix = new QPixmap(780, 430);
	backgroundPix->load("c:/cssd/template/background_w.jpg");
	painter->drawPixmap(0, 0, *backgroundPix);

	//draw text
	QFont titleFont("黑体", 24, QFont::Bold);
	painter->setFont(titleFont);
	painter->drawText(180, 70, label.sterilizeName);
	QFont txtFont("黑体", 18, QFont::Bold);
	painter->setFont(txtFont);
	painter->drawText(180, 120, QString::fromLocal8Bit("灭菌日期:%1").arg(label.sterilizeDate));
	painter->drawText(180, 170, QString::fromLocal8Bit("灭菌时间:%1").arg(label.sterilizeTime));
	painter->drawText(180, 220, QString::fromLocal8Bit("第%1锅次，共%2包").arg(label.panNum).arg(label.packageNum));

	//draw barcode
	bc.setText(label.sterilizeId);
	bc.setBorderType(Zint::QZint::NO_BORDER);
	bc.setHideText(true);

	bc.render(*painter, QRectF(180, 240, 400, 150), Zint::QZint::IgnoreAspectRatio);
	painter->drawText(240, 400, label.sterilizeId);

	painter->end();

	//save file
	QString filePath = QString("C:/cssd/%1.jpg").arg(label.sterilizeId);
	QFile file(filePath);
	file.open(QIODevice::WriteOnly);
	pix->save(&file, "JPG");

	return OK;
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
