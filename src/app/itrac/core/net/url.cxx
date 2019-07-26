#include "url.h"

#include <QVariantMap>

#ifdef SELL_DEMO
const char PATH_BASE[] = "http://120.76.128.123:8082/CSSD/";
#endif //SELL_DEMO


#ifdef LOCAL_DEMO
const char PATH_BASE[] = "http://localhost:8080/CSSD/";
#endif //LOCAL_DEMO


const char PATH_PLATE_SEARCH[] = "plate/search";
const char PATH_DEPT_SEARCH[] = "department/search";
const char PATH_DEVICE_ADD[] = "device/add";
const char PATH_DEVICE_SEARCH[] = "device/search";
const char PATH_DEVICE_MODIFY[] = "device/modify";
const char PATH_DEVICE_STOP[] = "device/cancelRunning";
const char PATH_PROGRAM_ADD[] = "program/add";
const char PATH_PROGRAM_SEARCH[] = "program/search";
const char PATH_PROGRAM_MODIFY[] = "program/modify";
const char PATH_PROGRAM_SUPPORT[] = "program/support";
const char PATH_USER_LOGIN[] = "user/login";
const char PATH_USER_MODIFY_PWD[] = "user/modifyPassword";
const char PATH_USER_ADD[] = "user/add";
const char PATH_USER_SEARCH[] = "user/search";
const char PATH_PKG_INFO[] = "package/info";
const char PATH_RECYCLE_ADD[] = "recycle/add";
const char PATH_RECYCLE_SEARCH[] = "recycle/search";
const char PATH_PKGTPYE_SEARCH[] = "packageType/search";
const char PATH_PKGDETAIL_SEARCH[] = "packageType/searchInstruments";
const char PATH_INSTRUMENT_SEARCH[] = "instrument/search";
const char PATH_INSTRUMENT_ADD[] = "instrument/add";
const char PATH_PKG_IN_PLATE[] = "pack/packageType";
const char PATH_WASH_ADD[] = "wash/add";
const char PATH_WASH_CHECK[] = "wash/check";
const char PATH_WASH_SEARCH[] = "wash/search";
const char PATH_PACK_ADD[] = "pack/add";
const char PATH_PACK_SEARCH[] = "pack/search";
const char PATH_PACKTYPE_SEARCH[] = "packType/search";
const char PATH_STERILE_ADD[] = "sterilize/add";
const char PATH_STERILE_INFO[] = "sterilize/info";
const char PATH_STERILE_CHECK[] = "sterilize/addPlateTest";
const char PATH_STERILE_SEARCH[] = "sterilize/search";
const char PATH_BD_ADD[] = "bd/add";
const char PATH_ISSUE_ADD[] = "issue/add";
const char PATH_ISSUE_SEARCH[] = "issue/search";
const char PATH_TRACE_PACKAGE[] = "trace/package";
const char PATH_TRACE_PATIENT[] = "trace/patient";

const char PATH_ORDER_ADD[] = "order/add";
const char PATH_ORDER_SEARCH[] = "order/search";
const char PATH_ORDER_RECYCLE[] = "order/recycle";
const char PATH_ORDER_PKGINFO[] = "order/pkgInfo";

const char PATH_EXT_ADD[] = "ext/add";
const char PATH_EXTORDER_SEARCH[] = "ext/searchExtOrder";
const char PATH_EXT_SEARCH[] = "ext/searchExt";
const char PATH_EXT_RETURN[] = "ext/return";

const char PATH_VENDOR_SEARCH[] = "supplier/search";

const char PATH_PACKTYPE_GETCOUNT[] = "statistics/countPackType";
const char PATH_COST_GETMONTH[] = "cost/getMonth";
const char PATH_COST_GETALL[] = "cost/costAll";
const char PATH_COST_SEARCH[] = "cost/search";
const char PATH_COST_ADD[] = "cost/add";
const char PATH_PRICE_ADD[] = "price/add";

const char PATH_WARN_PKGS[] = "warning/warningPackage";
const char PATH_RECALL[] = "warning/recall";
const char PATH_RECALL_SEARCH[] = "warning/recallSearch";

const char PATH_VERSION[] = "file/version/itrac";
