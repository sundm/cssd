#include "imgprinter.h"
#include <qpainter.h>
#include <qpixmap.h>
#include <qfile.h>

ImgPrinter::ImgPrinter():LabelPrinter()
{
}

ImgPrinter::~ImgPrinter()
{
}


void ImgPrinter::setFontName(const QString &strFontName)
{

}
void ImgPrinter::setInversion(const bool inver) {

}
void ImgPrinter::setBold(const bool isBold) {

}

int ImgPrinter::buildPackageLabels(const PackageLabel &label, QString &szLabelBuilder)
{
	QPixmap *pix = new QPixmap(780, 600);
	QPainter *painter = new QPainter(pix);
	pix->fill(Qt::transparent);

	painter->begin(pix);

	//draw background
	QPixmap *backgroundPix = new QPixmap(780, 600);
	backgroundPix->load("c:/cssd/template/background_h.jpg");	
	painter->drawPixmap(0, 0, *backgroundPix);
	
	//draw text
	QFont titleFont("����", 24, QFont::Bold);
	painter->setFont(titleFont);
	painter->drawText(180, 70, label.packageName);
	QFont txtFont("����", 18, QFont::Bold);
	painter->setFont(txtFont);
	QString packageTpye = QString("%1[%2]").arg(label.packageFrom).arg(label.packageType);
	painter->drawText(180, 120, packageTpye);
	painter->drawText(180, 170, QString::fromLocal8Bit("�������:%1").arg(label.disinDate));
	painter->drawText(180, 220, QString::fromLocal8Bit("ʧЧ����:%1").arg(label.expiryDate));
	painter->drawText(180, 270, QString::fromLocal8Bit("���ռ���:%1").arg(QString::number(label.count)));
	painter->drawText(180, 320, QString::fromLocal8Bit("�����:%1").arg(label.operatorName));
	painter->drawText(500, 320, QString::fromLocal8Bit("�����:%1").arg(label.assessorName));
	
	//draw barcode
	bc.setText(label.packageId);
	bc.setBorderType(Zint::QZint::NO_BORDER);
	bc.setHideText(true);
	
	bc.render(*painter, QRectF(150, 360, 500, 200),Zint::QZint::IgnoreAspectRatio);
	painter->drawText(240, 570, label.packageId);

	painter->end();
	
	//save file
	QString filePath = QString("C:/cssd/%1.jpg").arg(label.packageId);
	QFile file(filePath);
	file.open(QIODevice::WriteOnly);
	pix->save(&file, "JPG");
	return OK;
}

int ImgPrinter::buildClinicLabels(const ClinicLabel &label, QString &szLabelBuilder)
{
	QPixmap *pix = new QPixmap(780, 430);
	QPainter *painter = new QPainter(pix);
	pix->fill(Qt::transparent);

	painter->begin(pix);

	//draw background
	QPixmap *backgroundPix = new QPixmap(780, 430);
	backgroundPix->load("c:/cssd/template/background_w.jpg");	
	painter->drawPixmap(0, 0, *backgroundPix);
	
	//draw text
	QFont titleFont("����", 24, QFont::Bold);
	painter->setFont(titleFont);
	painter->drawText(180, 70, label.packageName);
	QFont txtFont("����", 18, QFont::Bold);
	painter->setFont(txtFont);
	painter->drawText(180, 120, QString::fromLocal8Bit("����:%1").arg(label.packageFrom));
	painter->drawText(180, 160, QString::fromLocal8Bit("�������:%1").arg(label.disinDate));
	painter->drawText(180, 200, QString::fromLocal8Bit("ʧЧ����:%1").arg(label.expiryDate));
	painter->drawText(180, 240, QString::fromLocal8Bit("�����:%1").arg(label.operatorName));
	painter->drawText(500, 240, QString::fromLocal8Bit("�����:%1").arg(label.assessorName));
	
	//draw barcode
	bc.setText(label.packageId);
	bc.setBorderType(Zint::QZint::NO_BORDER);
	bc.setHideText(true);
	
	bc.render(*painter, QRectF(180, 260, 400, 150),Zint::QZint::IgnoreAspectRatio);
	painter->drawText(240, 420, label.packageId);

	painter->end();
	
	//save file
	QString filePath = QString("C:/cssd/%1.jpg").arg(label.packageId);
	QFile file(filePath);
	file.open(QIODevice::WriteOnly);
	pix->save(&file, "JPG");

	return OK;
}

int ImgPrinter::buildSterilizedLabels(const SterilizeLabel &label, QString &szLabelBuilder)
{
	QPixmap *pix = new QPixmap(780, 430);
	QPainter *painter = new QPainter(pix);
	pix->fill(Qt::transparent);

	painter->begin(pix);

	//draw background
	QPixmap *backgroundPix = new QPixmap(780, 430);
	backgroundPix->load("c:/cssd/template/background_w.jpg");
	painter->drawPixmap(0, 0, *backgroundPix);

	//draw text
	QFont titleFont("����", 24, QFont::Bold);
	painter->setFont(titleFont);
	painter->drawText(180, 70, label.sterilizeName);
	QFont txtFont("����", 18, QFont::Bold);
	painter->setFont(txtFont);
	painter->drawText(180, 120, QString::fromLocal8Bit("�������:%1").arg(label.sterilizeDate));
	painter->drawText(180, 170, QString::fromLocal8Bit("���ʱ��:%1").arg(label.sterilizeTime));
	painter->drawText(180, 220, QString::fromLocal8Bit("��%1���Σ���%2��").arg(label.panNum).arg(label.packageNum));

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