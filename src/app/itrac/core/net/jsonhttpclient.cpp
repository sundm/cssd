#include <QDebug>
#include "jsonhttpclient.h"
#include <thirdparty/qjson/src/serializer.h>

JsonHttpClient::JsonHttpClient()
{
}

JsonHttpClient::~JsonHttpClient()
{
}

void JsonHttpClient::post(const QString &url, const QVariantMap &vmap,
	const std::function<void(QNetworkReply *)> &finishCallback)
{
	QJson::Serializer serializer;
	bool ok;
	QByteArray payload = serializer.serialize(vmap, &ok);

	if (!ok) {
		// TODO, Internal error, this should never happen
		qCritical() << "Something went wrong:" << serializer.errorMessage();
	}

	post(url, payload, finishCallback);
}

const QByteArray JsonHttpClient::post(const QString &url, QHttpMultiPart *multiPart) {
	return XHttpClient::post(url, multiPart);
}

const QByteArray JsonHttpClient::post(const QString &url, const QByteArray &data) {
	qDebug() << ">>> " << data;
	setHeader("content-type", "application/json");
	return XHttpClient::post(url, data);
}

void JsonHttpClient::post(const QString &url, const QByteArray &data,
	const std::function<void(QNetworkReply *)> &finishCallback,
	const std::function<void(const QString &)> &errorCallback/* = nullptr*/)
{
	qDebug() << ">>> " << data;
	setHeader("content-type", "application/json");
	XHttpClient::post(url, data, finishCallback, errorCallback);
}
