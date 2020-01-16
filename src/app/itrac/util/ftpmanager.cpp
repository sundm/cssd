#include <QFileInfo>
#include "ftpmanager.h"

FtpManager* FtpManager::_instance = NULL;

FtpManager *FtpManager::getInstance() {
	if (NULL == _instance) {
		_instance = new FtpManager();
	}

	return _instance;
}

FtpManager::FtpManager(QObject *parent)
	: QObject(parent)
{
	m_pUrl.setScheme("ftp");
}

void FtpManager::setHostPort(const QString &host, int port)
{
	m_pUrl.setHost(host);
	m_pUrl.setPort(port);
}

void FtpManager::setUserInfo(const QString &userName, const QString &password)
{
	m_pUrl.setUserName(userName);
	m_pUrl.setPassword(password);
}

void FtpManager::put(const QString &fileName, const QString &path)
{
	m_file.setFileName(fileName);
	m_file.open(QIODevice::ReadOnly);
	QByteArray data = m_file.readAll();

	m_pUrl.setPath(path);
	QNetworkReply *pReply = m_manager.put(QNetworkRequest(m_pUrl), data);

	connect(pReply, SIGNAL(finished()), this, SLOT(onUploadFinished()));
	connect(pReply, SIGNAL(uploadProgress(qint64, qint64)), this, SIGNAL(uploadProgress(qint64, qint64)));
	connect(pReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SIGNAL(error(QNetworkReply::NetworkError)));
}

void FtpManager::get(const QString &path, const QString &fileName)
{
	
	m_file.open(QIODevice::WriteOnly | QIODevice::Append);
	m_pUrl.setPath(path);

	QNetworkReply *pReply = m_manager.get(QNetworkRequest(m_pUrl));

	connect(pReply, SIGNAL(finished()), this, SLOT(onDownloadFinished()));
	connect(pReply, SIGNAL(downloadProgress(qint64, qint64)), this, SIGNAL(downloadProgress(qint64, qint64)));
	connect(pReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
}

void FtpManager::onError(QNetworkReply::NetworkError e)
{
	m_file.close();

	emit error(e);
}

void FtpManager::onUploadFinished()
{
	m_file.close();

	emit uploadFinished();
}


void FtpManager::onDownloadFinished()
{
	QNetworkReply *pReply = qobject_cast<QNetworkReply *>(sender());
	switch (pReply->error()) 
	{
	case QNetworkReply::NoError: 
	{
		m_file.write(pReply->readAll());
		m_file.flush();

		emit downloadFinished();
		break;
	}
	default:
		break;
	}

	m_file.close();
	pReply->deleteLater();
}