#include <QURL>
#include <QDEBUG>
#include <QNetworkReply>
#include <QThread>
#include <QMessageBox>
#include <thirdparty/qjson/src/parser.h>
#include <qscanner/qscanner.h>
#include "loginwidget.h"
#include <xernel/xhttpclient.h>
#include <xernel/xhttpresponse.h>
#include "waitingspinnerwidget.h"
#include "urls.h"

LoginWidget::LoginWidget(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);

	edit_psw->setMaxLength(16);

	updateReaderMenu();

	setFocusPolicy(Qt::StrongFocus);
	setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

	// HID-KBW mode
	QScanner *scanner = QScannerFactory::Create(QScannerFactory::NLOY, NULL);
	connect(scanner, SIGNAL(dataRecieved(const QString&)), this, SLOT(onBarcodeScanned(const QString&)), Qt::QueuedConnection);
	scanner->start();

	// HID-KBW mode, need installation of event filter on QApplication/widgets
	//ScanEater *se = new ScanEater(this);
	//installEventFilter(se);
	//connect(se, SIGNAL(dataReceived(const QString&)), this, SLOT(onBarcodeScanned(const QString&)));
}

LoginWidget::~LoginWidget()
{
}

void LoginWidget::updateReaders()
{
	updateReaderMenu();
}

void LoginWidget::updateReaderMenu()
{
}

void LoginWidget::showInfo(const QString & info, bool isError)
{
	//label_info->setText(info);
	//if (isError){
	//	label_info->setStyleSheet("color: red");
	//}
	//else{
	//	label_info->setStyleSheet("color: green");
	//}
}

void LoginWidget::onThreadFinished()
{
	//if (!_thread->hasError()){
	//	frame->setEnabled(true);
	//}
}

void LoginWidget::onBarcodeScanned(const QString &barcode)
{
	qDebug() << barcode;
}

void LoginWidget::onLoginFinished(QNetworkReply* reply)
{
	XHttpResponse resp(reply);
	if (resp.success()) {
		QDialog::accept();
	}
	else {
		QMessageBox::warning(this, "Login", "Invalid username or password");
	}
}

void LoginWidget::accept()
{
	QDialog::accept();
	return;
	WaitingSpinnerWidget* spinner = new WaitingSpinnerWidget(this->frame);
	spinner->start();

	QVariantMap vmap;
	vmap.insert("operator_name", edit_user->text());
	vmap.insert("operator_pwd", edit_psw->text());

	/*XHttpClient *request = new XHttpClient(this);
	connect(request, SIGNAL(finished(QNetworkReply*)), this, SLOT(onLoginFinished(QNetworkReply*)));
	connect(request, SIGNAL(finished(QNetworkReply*)), spinner, SLOT(stop()));
	request->post(url(PATH_USER_LOGIN), vmap);

	return;*/


#ifdef QT_NO_DEBUG
	//Opc::Operator &op = qApp->theOperator();
	//if (!op.identified()){
	//	showInfo("请连接操作员卡");
	//	return;
	//}

	//QString pin(edit_psw->text());
	//if (pin.isEmpty()){
	//	showInfo("请输入密码");
	//	return;
	//}

	//if (pin.length() != 6){
	//	showInfo("密码长度必须是6位");
	//	return;
	//}

	//stopTimer();

	//int kfpc = 0xFF;
	//int ret = op.login(pin, &kfpc);
	//if (ret){
	//	if (OPC_USER_PIN_WRONG == ret && kfpc > 0 && kfpc < 0x0F)
	//		QMessageBox::critical(this, "错误", tr("密码错误，在卡片锁定之前您还有%1次机会").arg(kfpc));
	//	else
	//		QMessageBox::critical(this, "错误", err2string(ret));

	//	startTimer();
	//	return;
	//}

	//qApp->recordSysLog(Opc::LogIn, "登进系统");
#endif

	QDialog::accept();
}
