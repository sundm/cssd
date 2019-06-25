#ifndef URLS_H
#define URLS_H

#include <QString>

extern const char URL_BASE[];

extern const char PATH_PLATE_SEARCH[];
extern const char PATH_USER_SEARCH[];
extern const char PATH_DEPT_SEARCH[];
extern const char PATH_DEVICE_SEARCH[];
extern const char PATH_DEVICE_MODIFY[];
extern const char PATH_PROGRAM_SEARCH[];
extern const char PATH_PROGRAM_MODIFY[];
extern const char PATH_PROGRAM_SUPPORT[];
extern const char PATH_USER_LOGIN[];
extern const char PATH_PKG_INFO[];
extern const char PATH_RECYCLE_ADD[];
extern const char PATH_RECYCLE_SEARCH[];
extern const char PATH_PKGTPYE_SEARCH[];
extern const char PATH_PKG_IN_PLATE[];
extern const char PATH_WASH_ADD[];
extern const char PATH_WASH_CHECK[];
extern const char PATH_PACK_ADD[];
extern const char PATH_STERILE_ADD[];
extern const char PATH_STERILE_INFO[];
extern const char PATH_STERILE_CHECK[];
extern const char PATH_ISSUE_ADD[];

inline const QString url(const char *path) {
	return QString(URL_BASE).append(path);
}

#endif // URLS_H
