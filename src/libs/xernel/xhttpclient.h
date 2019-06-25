#ifndef XHTTPCLIENT_H
#define XHTTPCLIENT_H

#include "xernel_global.h"
#include <functional>
#include <QScopedPointer>

QT_FORWARD_DECLARE_CLASS(XHttpClientPrivate)
QT_FORWARD_DECLARE_CLASS(QNetworkReply)

class XERNEL_EXPORT XHttpClient
{
	X_DECLARE_SIMPLE_PRIVATE(XHttpClient)

public:
	XHttpClient();
	virtual ~XHttpClient();

	void setHeader(const QString &headerName, const QString &headerValue);

	// asynchronous post method
	virtual void post(
		const QString &url,
		const QByteArray &data,
		const std::function<void(QNetworkReply *)> &finishCallback,
		const std::function<void(const QString &)> &errorCallback = nullptr);

	// synchronous post method
	QNetworkReply *post(const QString &url, const QByteArray &data);

};

#endif // !XHTTPCLIENT_H

