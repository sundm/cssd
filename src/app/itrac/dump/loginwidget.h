#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <qdialog>
#include "ui_loginwidget.h"

class QTimer;
class QNetworkReply;
class LoginWidget : public QDialog, public Ui::LoginWidget
{
    Q_OBJECT
    
public:
    explicit LoginWidget(QWidget *parent = 0);
	~LoginWidget();

public:
    virtual void accept();
//protected:
	//bool eventFilter(QObject *obj, QEvent *event);

private slots:
	void updateReaders();
	void showInfo(const QString &info, bool isError= true);
	void onThreadFinished();
	void onBarcodeScanned(const QString &);
	void onLoginFinished(QNetworkReply*);

private:
	void updateReaderMenu();

private:
	QTimer *_timer;
};

#endif // LOGINWIDGET_H
