#pragma once

//#include "jsonhttpclient.h"
#include <QString>
#include "jsonhttpresponse.h"

extern QString PATH_BASE;
extern QString LABEL_PRINTER;
extern QString COMMON_PRINTER;
extern const char PATH_PLATE_SEARCH[];
extern const char PATH_DEPT_SEARCH[];
extern const char PATH_DEPT_ADD[];
extern const char PATH_DEPT_MODIFY[];
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
extern const char PATH_USER_RESET_PWD[];
extern const char PATH_USER_ADD[];
extern const char PATH_USER_MODIFY[];
extern const char PATH_USER_SEARCH[];
extern const char PATH_PKG_INFO[];
extern const char PATH_RECYCLE_ADD[];
extern const char PATH_RECYCLE_SEARCH[];
extern const char PATH_PKGTPYE_SEARCH[];
extern const char PATH_PKGTPYE_ADD[];
extern const char PATH_PKGTPYE_PKGIDS[];
extern const char PATH_PKGTPYE_MODIFY[];
extern const char PATH_PKGTPYE_UPLOAD_IMG[];
extern const char PATH_PKGTPYE_DOWNLOAD_IMG[];
extern const char PATH_PKGDETAIL_SEARCH[];
extern const char PATH_INSTRUMENT_SEARCH[];
extern const char PATH_INSTRUMENT_ADD[];
extern const char PATH_INSTRUMENT_INSIDS[];
extern const char PATH_INSTRUMENT_UPLOAD_IMG[];
extern const char PATH_INSTRUMENT_DOWNLOAD_IMG[];
extern const char PATH_INSTRUMENT_MODIFY[];
extern const char PATH_PKG_IN_PLATE[];
extern const char PATH_WASH_ADD[];
extern const char PATH_WASH_CHECK[];
extern const char PATH_WASH_SEARCH[];
extern const char PATH_WASH_INFO[];

extern const char PATH_PACK_ADD[];
extern const char PATH_PACK_SEARCH[];
extern const char PATH_PACK_REPRINT[];
extern const char PATH_PACKTYPE_SEARCH[];
extern const char PATH_PACKTYPE_ADD[];
extern const char PATH_PACKTYPE_MODIFY[];
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
extern const char PATH_EXTORDER_SEARCH[];
extern const char PATH_EXT_SEARCH[];
extern const char PATH_EXT_PROCESS[];
extern const char PATH_EXT_RETURN[];

extern const char PATH_VENDOR_SEARCH[];

extern const char PATH_PACKTYPE_GETCOUNT[];
extern const char PATH_COST_GETMONTH[];
extern const char PATH_COST_GETALL[];
extern const char PATH_COST_SEARCH[];
extern const char PATH_COST_ADD[];
extern const char PATH_PRICE_ADD[];
extern const char PATH_WARN_PKGS[];
extern const char PATH_RECALL[];
extern const char PATH_RECALL_SEARCH[];

extern const char PATH_OP_STATISTICS[];
extern const char PATH_DEPT_STATISTICS[];
extern const char PATH_WASH_STATISTICS[]; 
extern const char PATH_STERILE_STATISTICS[];

extern const char PATH_VERSION[];
extern const char PATH_UPDATE[];

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



