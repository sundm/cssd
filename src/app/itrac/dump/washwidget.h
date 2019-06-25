#ifndef WASHWIDGET_H
#define WASHWIDGET_H

#include "ui_washwidget.h"

class QStandardItemModel;
class XHttpClient;
class QNetworkReply;
class WashWidget : public QWidget, Ui::WashWidget
{
    Q_OBJECT

public:
    explicit WashWidget(QWidget *parent = 0);
    ~WashWidget();

private slots:
	void barcodeScanned(const QString &);
	void reset();
	void startWash();
	void updateProgramView(const QModelIndex &current, const QModelIndex &previous);
	void inputBarcode();

private:
	void initPackageTableView();
	void initDeviceTableView();
	void initProgramTableView();
	void showPlateDetails(const QString &);
	void clearPrograms();

private:
	QStandardItemModel * _pkgModel;
	QStandardItemModel * _deviceModel;
	QStandardItemModel * _programModel;
};

#endif // !WASHWIDGET_H
