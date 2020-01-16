#pragma once

#include <QUrl>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class FtpManager : public QObject
{
	Q_OBJECT

public:
	static FtpManager* getInstance();

	void setHostPort(const QString &host, int port = 21);

	void setUserInfo(const QString &userName, const QString &password);

	void put(const QString &fileName, const QString &path);

	void get(const QString &path, const QString &fileName);

signals:
	void downloadFinished();
	void uploadFinished();

	void error(QNetworkReply::NetworkError);

	void uploadProgress(qint64 bytesSent, qint64 bytesTotal);

	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private slots:
	void onDownloadFinished();
	void onUploadFinished();
	void onError(QNetworkReply::NetworkError);
private:
	explicit FtpManager(QObject *parent = 0);
	static FtpManager* _instance;
	QUrl m_pUrl;
	QFile m_file;
	QNetworkAccessManager m_manager;
};
