#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_winsupdate.h"
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>


class Winsupdate : public QMainWindow
{
	Q_OBJECT

public:
	Winsupdate(QString &path_url, QWidget *parent = Q_NULLPTR);
	QString server_url;

private slots:
	void replyFinished(QNetworkReply *reply);
	void httpDownloadFinished(QNetworkReply *reply);
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void writeToFile();
	void on_click_btn();
private:
	Ui::WinsupdateClass ui;
	QString zipFileName;
	QString zipFileMD5;
	QFile *zipFile;
	QTimer *timer;
	QNetworkAccessManager *netManager;
	QNetworkReply* mGetVerReply;
	QNetworkReply* mDownloadReply;
	QString getFileMd5(QString filePath);
	bool createFile();
};
