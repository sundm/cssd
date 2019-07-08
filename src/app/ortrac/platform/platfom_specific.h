#ifdef Q_OS_MAC
#include "platform/mac/launcher_mac.h"
#elif defined Q_OS_LINUX // Q_OS_MAC
#include "platform/linux/launcher_linux.h"
#elif defined Q_OS_WIN // Q_OS_MAC || Q_OS_LINUX
#include "platform/win/launcher_win.h"
#endif // Q_OS_MAC || Q_OS_LINUX || Q_OS_WIN