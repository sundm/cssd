#include "jsonhttpresponse.h"
#include <thirdparty/qjson/src/parser.h>

JsonHttpResponse::JsonHttpResponse(QNetworkReply *reply)
	:XHttpResponse(reply)
{
	if (XHttpResponse::success()) {
		QJson::Parser parser;
		bool ok;
		_vmap = parser.parse(data(), &ok).toMap();
		if (!ok) {
			setErrorString("Unrecognizable format of HTTP response");
		}
		else {
			QString code = _vmap.value("code").toString();
			//if ("00000" != code) {
			if ("9000" != code) {
				QVariant msg = _vmap.value("msg");
				setErrorString(msg.isValid() ?
					msg.toString() : QString("Unknown error with code[%1]").arg(code));
			}
		}
	}
}

JsonHttpResponse::~JsonHttpResponse()
{
}
