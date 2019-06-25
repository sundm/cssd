#include "xhttpresponse.h"
#include <qnetworkreply>

class XHttpResponsePrivate {
	X_DECLARE_SIMPLE_PUBLIC(XHttpResponse)
	QString error;
	QByteArray data;
};

XHttpResponse::XHttpResponse(QNetworkReply *reply)
	:d(new XHttpResponsePrivate)
{
	Q_ASSERT(reply->isFinished());

	if (reply->error() > 0) {
		d->error = reply->errorString();
	}
	else {
		int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (code >= 200 && code < 300) { // Success
			d->data = reply->readAll();
			qDebug() << "<<<" << d->data;
			qDebug() << "<<<" << QString(d->data);
		}
		else {
			d->error = QString("HTTP error, code=<%1>").arg(code);
		}
	}
}

XHttpResponse::~XHttpResponse()
{
}

bool XHttpResponse::success() const
{
	return d->error.isEmpty();
}

QString XHttpResponse::errorString() const
{
	return d->error;
}

void XHttpResponse::setErrorString(const QString &errorString)
{
	d->error = errorString;
}

/*
 *is this a good way to disable modification on the result?
 */
const QByteArray &XHttpResponse::data() const
{
	return d->data;
}
