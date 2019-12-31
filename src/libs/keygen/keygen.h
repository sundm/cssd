#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_keygen.h"

class keygen : public QMainWindow
{
	Q_OBJECT

public:
	keygen(QWidget *parent = Q_NULLPTR);

private slots:
	void onGenClicked();

private:
	Ui::keygenClass ui;
};
