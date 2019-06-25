#pragma once

#include <QWidget>
#include "ui_washcheckwidget.h"

class QStandardItemModel;
class WashCheckWidget : public QWidget, public Ui::WashCheckWidget
{
	Q_OBJECT

public:
	WashCheckWidget(QWidget *parent = Q_NULLPTR);
	~WashCheckWidget();

private slots:
	void barcodeScanned(const QString &);
	void commitWashQualified();
	void commitWashUnqualified();
	void doPack();

private:
	void initPackageView();
	void showPlateDetails(const QString &bc);
private:
	QStandardItemModel * _pkgModel;
};
