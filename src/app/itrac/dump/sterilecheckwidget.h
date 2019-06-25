#pragma once

#include <QWidget>
#include "ui_sterilecheckwidget.h"

class QStandardItemModel;
class SterileCheckWidget : public QWidget, public Ui::SterileCheckWidget
{
	Q_OBJECT

public:
	SterileCheckWidget(QWidget *parent = Q_NULLPTR);
	~SterileCheckWidget();

private slots:
	void barcodeScanned(const QString & bc);
	void commitSterileQualified();
	void commitSterileUnqualified();
	void updateSterileInfo(const QString &);

private:
	void initPackageView();

private:
	QStandardItemModel * _pkgModel;
	bool _chemInvolved;
	bool _bioInvolved;
};
