#pragma once

#include <QtWidgets/QWidget>
#include "ui_ortrac.h"

class ortrac : public QWidget
{
	Q_OBJECT

public:
	ortrac(QWidget *parent = Q_NULLPTR);

private:
	Ui::ortracClass ui;
};
