#include "url.h"

#include <QVariantMap>

namespace Url {
	const QString Version("V1.0.1");
	const char PATH_BASE[] = "http://120.76.128.123:8080/CSSD/";
	//const char PATH_BASE[] = "http://localhost:8080/CSSD/";
	const char PATH_PLATE_SEARCH[] = "plate/search";
	const char PATH_DEPT_SEARCH[] = "department/search";
	const char PATH_USER_LOGIN[] = "user/login";
	const char PATH_USER_MODIFY_PWD[] = "user/modifyPassword";
	const char PATH_PKG_INFO[] = "package/info";
	const char PATH_PKGTPYE_SEARCH[] = "packageType/search";
	const char PATH_PKG_IN_PLATE[] = "pack/packageType";
	const char PATH_TRACE_PACKAGE[] = "trace/package";
	const char PATH_TRACE_PATIENT[] = "trace/patient";

	const char PATH_USE_ADD[] = "use/add";

	const char PATH_VERSION[] = "file/version";
	const char PATH_UPDATE[] = "/";
} // namespace Url
