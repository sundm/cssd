#include "xhttpclient.h"
#include "xhttpresponse.h"
#include <qnetworkreply>
#include <QEventLoop>

XHttpClient::XHttpClient()
{
}

XHttpClient::~XHttpClient()
{
	QNetworkReply *reply = nullptr;
	for (int i = 0; i < _replies.size(); ++i) {
		reply = _replies.at(i);
		if (reply->isRunning()) {
			reply->disconnect(SIGNAL(finished()), 0, 0);
			reply->abort();
		}
		reply->deleteLater();
	}
}

void XHttpClient::setHeader(const QString &headerName, const QString &headerValue)
{
	_headers[headerName] = headerValue;
}

void XHttpClient::post(const QString &url,
	const QByteArray &data,
	const std::function<void(QNetworkReply *)> &finishCallback,
	const std::function<void(const QString &)> &errorCallback/* = nullptr*/)
{
	QUrl u(url);
	QNetworkRequest request(u);

	QHashIterator<QString, QString> iter(_headers);
	while (iter.hasNext()) {
		iter.next();
		request.setRawHeader(iter.key().toUtf8(), iter.value().toUtf8());
	}

	//clock_t startTime, endTime;
	//startTime = clock();

	// !!! DO NOT create the QNetworkAccessManager object on stack here, otherwise, 
	// it's destroyed immediately, cause the QNetworkAccessManager::finished signal never be emitted,
	// we should DELETE it later.
	QNetworkReply *reply = _nam.post(request, data);
	_replies.append(reply);

	QObject::connect(reply, &QNetworkReply::finished, [=] {
		_replies.removeAll(reply);
		if (nullptr != finishCallback) {
			finishCallback(reply);
		}
		reply->deleteLater();
	});
	if (nullptr != errorCallback) {
		QObject::connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), [=] {
			errorCallback(reply->errorString());
		});
	}
}

QNetworkReply * XHttpClient::post(const QString &url, const QByteArray &data)
{
	QUrl destUrl(url);
	QNetworkRequest request(destUrl);

	QHashIterator<QString, QString> iter(_headers);
	while (iter.hasNext()) {
		iter.next();
		request.setRawHeader(iter.key().toUtf8(), iter.value().toUtf8());
	}

	QNetworkAccessManager nam;
	QNetworkReply *reply = nam.post(request, data);

	// wait until finished, fake synchronous
	//while (!reply->isFinished()) {
	//	qApp->processEvents();
	//}

	// or use QEventLoop to wait, is this better?
	QEventLoop loop;
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();

	return reply;
}

const QByteArray XHttpClient::post(const QString &url, QHttpMultiPart *multiPart)
{
	QUrl destUrl(url);
	QNetworkRequest request(destUrl);

	QNetworkAccessManager nam;
	QNetworkReply *reply = nam.post(request, multiPart);

	QEventLoop loop;
	QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();

	XHttpResponse resp(reply);
	if (resp.success())
		return resp.data();
	else
		return nullptr;
}