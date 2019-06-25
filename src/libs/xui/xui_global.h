#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(XUI_LIB)
#  define XUI_EXPORT Q_DECL_EXPORT
# else
#  define XUI_EXPORT Q_DECL_IMPORT
# endif
#else
# define XUI_EXPORT
#endif
