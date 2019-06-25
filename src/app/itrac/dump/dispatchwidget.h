#pragma once

#include <QWidget>
#include "ui_dispatchwidget.h"

class QStandardItemModel;
class DispatchWidget : public QWidget, Ui::DispatchWidget
{
	Q_OBJECT

public:
	DispatchWidget(QWidget *parent = Q_NULLPTR);
	~DispatchWidget();

private slots:
	void barcodeScanned(const QString &);
	void pickDept();
	void updateDept(const QString &);
	void updateDept(int, const QString &);
	void updateUser(const QString &);
	void updatePackageView(const QString &);
	void doDispatch();
	void reset();

private:
	void initPackageView();

private:
	QStandardItemModel * _pkgModel;
};
