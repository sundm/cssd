#pragma once

#include "ui/container.h"
#include "ui/loader.h"
#include <QWidget>
#include <qscanner/qscanner.h>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class Picture;
	class Container;
	class FlatEdit;
	class ErrorLabel;
} // namespace Ui 

class WaitingSpinner;
class QProgressBar;

class LoginPanel : public Ui::Inner, public Scanable, public JsonHttpClient
{
	Q_OBJECT
public:
	LoginPanel(Ui::Container *container = nullptr);
	void submit() override;
	

protected:
	void handleBarcode(const QString &) override;

private slots:
	void showRfidConfiger();
	void onBarcodeReceviced(const QString& code);
private:
	void login(const int, const QString &);
	void version();

	const QString getFileMd5(QString filePath);

	void getPkgImgs();
	void getInsImgs();

	void downloadPkgImgs();
	void downloadInsImgs();

	Ui::FlatEdit *userEdit;
	Ui::FlatEdit *pwdEdit;
	Ui::ErrorLabel *error;
	QProgressBar *bar;
	WaitingSpinner *_waiter;

	QList<QVariant> _pkg_ids;
	QList<QVariant> _ins_ids;
};

class StartupThread;
class LoginPage : public Ui::Source
{
	Q_OBJECT

public:
	LoginPage(QWidget *parent = nullptr);

signals:
	void permitted();

private:
	Ui::Picture * _headPic;
	Ui::Container *_container;

	StartupThread *_thread;
};

