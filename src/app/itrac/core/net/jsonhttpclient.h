#ifndef JSONHTTPCLIENT_H
#define JSONHTTPCLIENT_H

#include <xernel/xhttpclient.h>
#include <QVariantMap>

class QNetworkReply;
class JsonHttpClient : public XHttpClient
{
public:
	JsonHttpClient();
	~JsonHttpClient();

	void post(
		const QString &url,
		const QVariantMap &vmap,
		const std::function<void(QNetworkReply *)> & finishCallback);

	const QByteArray post(const QString &url, QHttpMultiPart *multiPart);

	const QByteArray post(const QString &url, const QByteArray &data);

	void post(
		const QString &url,
		const QByteArray &data,
		const std::function<void(QNetworkReply *)> & finishCallback,
		const std::function<void(const QString &)> & errorCallback = nullptr) override;
};

#endif // !JsonHttpClient
