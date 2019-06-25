#include "xhttpclient.h"
#include <qnetworkreply>
#include <QEventLoop>

class XHttpClientPrivate
{
	X_DECLARE_SIMPLE_PUBLIC(XHttpClient)
	//Q_DISABLE_COPY(XHttpClientPrivate)
	QHash<QString, QString> headers;
};


XHttpClient::XHttpClient()
	:d(new XHttpClientPrivate)
{
}

XHttpClient::~XHttpClient()
{
}

void XHttpClient::setHeader(const QString &headerName, const QString &headerValue)
{
	d->headers[headerName] = headerValue;
}

void XHttpClient::post(const QString &url,
	const QByteArray &data,
	const std::function<void(QNetworkReply *)> &finishCallback,
	const std::function<void(const QString &)> &errorCallback/* = nullptr*/)
{
	QUrl u(url);
	QNetworkRequest request(u);

	QHashIterator<QString, QString> iter(d->headers);
	while (iter.hasNext()) {
		iter.next();
		request.setRawHeader(iter.key().toUtf8(), iter.value().toUtf8());
	}

	//clock_t startTime, endTime;
	//startTime = clock();

	// !!! DO NOT create the QNetworkAccessManager object on stack here, otherwise, 
	// it's destroyed immediately, cause the QNetworkAccessManager::finished signal never be emitted,
	// we should DELETE it later.
	QNetworkAccessManager *nam = new QNetworkAccessManager;
	QNetworkReply *reply = nam->post(request, data);

	//endTime = clock();
	//qDebug() << endTime - startTime;

	// call finish handler
	QObject::connect(reply, &QNetworkReply::finished, [=] {
		if (nullptr != finishCallback) {
			finishCallback(reply);
		}
		reply->deleteLater();
		nam->deleteLater();
	});

	// call error handler if neccessary.
	// to check if a std::function is callable, simply use `if(func)` since
	// the template class defines `operator bool`, this is equivalant to
	// `if(func != nullptr)`, std::function also defines `operator==` and
	// `operator!=` overloads for comparing with a nullptr_t.
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

	QHashIterator<QString, QString> iter(d->headers);
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
