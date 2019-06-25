#ifndef RECYCLEWIDGET_H
#define RECYCLEWIDGET_H

#include "ui_recyclewidget.h"

class QStandardItemModel;
class RecycleWidget : public QWidget, Ui::RecycleWidget
{
    Q_OBJECT

public:
    explicit RecycleWidget(QWidget *parent = 0);
    ~RecycleWidget();

private slots:
	void barcodeScanned(const QString &);
	void reset();
	void inputBarcode();
	void selectPackage();
	void doRecycle();
	void insertNoBarcodePackage(const QString &, int, const QString &, int);

private:
	void initPackageTableView();
	void updatePlate(const QString &);
	void updatePackageView(const QString &);
	void updateUser(const QString &);

private:
	QStandardItemModel * _pkgModel;
};

#endif // !RECYCLEWIDGET_H
