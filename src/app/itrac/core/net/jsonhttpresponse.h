#ifndef JSONRESPONSE_H
#define JSONRESPONSE_H

#include <QVariantMap>
#include <xernel/xhttpresponse.h>

class JsonHttpResponse : public XHttpResponse
{
public:
	JsonHttpResponse(QNetworkReply *);
	~JsonHttpResponse();

	QString getAsString(const QString &key) { return _vmap[key].toString(); }
	int getAsInt(const QString &key) { return _vmap[key].toInt(); }
	bool getAsBool(const QString &key) { return _vmap[key].toBool(); }
	QVariantList getAsList(const QString &key) { return _vmap[key].toList(); }
	QVariantMap getAsDict(const QString &key) { return _vmap[key].toMap(); }

private:
	QVariantMap _vmap;
};

#endif // !JSONRESPONSE_H
