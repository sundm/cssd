#pragma once

//#include "jsonhttpclient.h"
#include "jsonhttpresponse.h"

#include <QString>

class QNetworkReply;

extern const QString Version;
extern QString PATH_BASE;
extern const char PATH_PLATE_SEARCH[];
extern const char PATH_DEPT_SEARCH[];
extern const char PATH_USER_LOGIN[];
extern const char PATH_USER_MODIFY_PWD[];
extern const char PATH_PKG_INFO[];
extern const char PATH_PKGTPYE_SEARCH[];
extern const char PATH_PKG_IN_PLATE[];
extern const char PATH_TRACE_PACKAGE[];
extern const char PATH_TRACE_PATIENT[];

extern const char PATH_ISSUE_SEARCH[];
extern const char PATH_PKG_PUSH[];

extern const char PATH_USE_ADD[];

extern const char PATH_VERSION[];

inline QString url(const char *path) {
	return QString(PATH_BASE).append(path);
}

//template<typename Data>
//void post(
//	const char path[],
//	const Data &data,
//	const std::function<void(QNetworkReply *)> &doneCallback) {
//	JsonHttpClient().post(url(path), data, doneCallback);
//}
