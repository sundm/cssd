#ifndef STERILEWIDGET_H
#define STERILEWIDGET_H

#include "ui_sterilewidget.h"

class QStandardItemModel;
class XHttpClient;
class QNetworkReply;
class SterileWidget : public QWidget, Ui::SterileWidget
{
    Q_OBJECT

public:
    explicit SterileWidget(QWidget *parent = 0);
    ~SterileWidget();

private slots:
	void barcodeScanned(const QString &);
	void reset();
	void startSterile();
	void updateProgramView(const QModelIndex &current, const QModelIndex &previous);

private:
	void initPackageTableView();
	void initDeviceTableView();
	void initProgramTableView();
	void updatePackageView(const QString &);
	void clearPrograms();

private:
	QStandardItemModel * _pkgModel;
	QStandardItemModel * _deviceModel;
	QStandardItemModel * _programModel;
};

#endif // !STERILEWIDGET_H
