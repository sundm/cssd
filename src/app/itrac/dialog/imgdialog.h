#pragma once

#include <QDialog>

class QLabel;

class ImgDialog : public QDialog
{
	Q_OBJECT

public:
	ImgDialog(QWidget *parent, const QString& imgPath);
	~ImgDialog();

private:
	void load();
	QString _imgPath;
	QLabel* _imgLabel;
};
