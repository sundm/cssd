#include "imgdialog.h"
#include <QLabel>
#include <QGridLayout>

ImgDialog::ImgDialog(QWidget *parent, const QString& imgPath)
	: QDialog(parent)
	, _imgLabel(new QLabel())
{
	setWindowTitle("器械图片");
	resize(parent->width()/3*2, 600);

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(_imgLabel);

	_imgPath = QString(imgPath);

	load();
}

ImgDialog::~ImgDialog() {
}

void ImgDialog::load() {
	QImage* img = new QImage();
	QImage* scaledimg = new QImage();
	if (!(img->load(_imgPath))) //加载图像
	{
		delete img;
		return;
	}
	//int Owidth = img->width(), Oheight = img->height();
	//float Fwidth, Fheight;       
	//int w = 700, h = 450;
	//_imgLabel->setGeometry(0, 0, w, h);

	//float Mul;          
	//if (Owidth / w >= Oheight / h)
	//	Mul = float(Owidth * 1.0f / w);
	//else
	//	Mul = float(Oheight * 1.0f / h);

	//Fwidth = Owidth / Mul;
	//Fheight = Oheight / Mul;
	//*scaledimg = img->scaled(ceil(Fwidth), ceil(Fheight), Qt::KeepAspectRatio);
	//_imgLabel->setPixmap(QPixmap::fromImage(*scaledimg));
	_imgLabel->setPixmap(QPixmap::fromImage(*img));}
