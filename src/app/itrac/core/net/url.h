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
extern const char PATH_DEVICE_ADD[];
extern const char PATH_DEVICE_SEARCH[];
extern const char PATH_DEVICE_MODIFY[];
extern const char PATH_DEVICE_STOP[];
extern const char PATH_PROGRAM_ADD[];
extern const char PATH_PROGRAM_SEARCH[];
extern const char PATH_PROGRAM_MODIFY[];
extern const char PATH_PROGRAM_SUPPORT[];
extern const char PATH_USER_LOGIN[];
extern const char PATH_USER_MODIFY_PWD[];
extern const char PATH_USER_ADD[];
extern const char PATH_USER_SEARCH[];
extern const char PATH_PKG_INFO[];
extern const char PATH_RECYCLE_ADD[];
extern const char PATH_RECYCLE_SEARCH[];
extern const char PATH_PKGTPYE_SEARCH[];
extern const char PATH_INSTRUMENT_SEARCH[];
extern const char PATH_INSTRUMENT_ADD[];
extern const char PATH_PKG_IN_PLATE[];
extern const char PATH_WASH_ADD[];
extern const char PATH_WASH_CHECK[];
extern const char PATH_WASH_SEARCH[];
extern const char PATH_PACK_ADD[];
extern const char PATH_PACK_SEARCH[];
extern const char PATH_STERILE_ADD[];
extern const char PATH_STERILE_INFO[];
extern const char PATH_STERILE_CHECK[];
extern const char PATH_STERILE_SEARCH[];
extern const char PATH_BD_ADD[];
extern const char PATH_ISSUE_ADD[];
extern const char PATH_ISSUE_SEARCH[];
extern const char PATH_TRACE_PACKAGE[];
extern const char PATH_TRACE_PATIENT[];

extern const char PATH_ORDER_ADD[];
extern const char PATH_ORDER_SEARCH[];
extern const char PATH_ORDER_RECYCLE[];
extern const char PATH_ORDER_PKGINFO[];

extern const char PATH_EXT_ADD[];
extern const char PATH_EXT_SEARCH[];
extern const char PATH_EXT_RETURN[];

extern const char PATH_VENDOR_SEARCH[];

extern const char PATH_PACKTYPE_GETCOUNT[];
extern const char PATH_COST_GETMONTH[];
extern const char PATH_COST_GETALL[];
extern const char PATH_COST_SEARCH[];
extern const char PATH_COST_ADD[];
extern const char PATH_PRICE_ADD[];

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


