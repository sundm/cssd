#ifndef XHTTPRESPONSE_H
#define XHTTPRESPONSE_H

#include "xernel_global.h"
#include <QScopedPointer>

class QNetworkReply;
class XHttpResponsePrivate;
class XERNEL_EXPORT XHttpResponse
{
	X_DECLARE_SIMPLE_PRIVATE(XHttpResponse)

public:
    explicit XHttpResponse(QNetworkReply *reply);
    ~XHttpResponse();

	bool success() const;
	QString errorString() const;
	void setErrorString(const QString &);
	const QByteArray &data() const;
};

#endif // !XHTTPRESPONSE_H
