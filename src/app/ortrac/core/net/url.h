#pragma once

#include "jsonhttpclient.h"
#include "jsonhttpresponse.h"

#include <QString>

class QNetworkReply;

namespace Url {

extern const QString Version;
extern const char PATH_BASE[];
extern const char PATH_PLATE_SEARCH[];
extern const char PATH_DEPT_SEARCH[];
extern const char PATH_USER_LOGIN[];
extern const char PATH_USER_MODIFY_PWD[];
extern const char PATH_PKG_INFO[];
extern const char PATH_PKGTPYE_SEARCH[];
extern const char PATH_PKG_IN_PLATE[];
extern const char PATH_TRACE_PACKAGE[];
extern const char PATH_TRACE_PATIENT[];

extern const char PATH_USE_ADD[];

extern const char PATH_VERSION[];
extern const char PATH_UPDATE[];

inline QString url(const char *path) {
	return QString(PATH_BASE).append(path);
}

template<typename Data>
void post(
	const char path[],
	const Data &data,
	const std::function<void(QNetworkReply *)> &doneCallback) {
	JsonHttpClient().post(url(path), data, doneCallback);
}

} // namespace Url


