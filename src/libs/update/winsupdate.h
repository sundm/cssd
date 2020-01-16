#pragma once

#include <QtWidgets/QMainWindow>
#include <QNetworkReply>
#include "ui_winsupdate.h"

class StartupThread;
class Winsupdate : public QMainWindow
{
	Q_OBJECT

public:
	Winsupdate(QWidget *parent = Q_NULLPTR);
	
private slots:
	void uploadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void uploadError(QNetworkReply::NetworkError);
	void uploadFinished();

	void on_update_btn();
	void on_file_btn();
private:
	Ui::WinsupdateClass ui;
	bool isSuccess;

	StartupThread *_thread;

	QString _fileMd5;
	QString _filePath;

	QString getFileMd5(QString filePath);
};
