#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(RCORE_LIB)
#  define RCORE_EXPORT Q_DECL_EXPORT
# else
#  define RCORE_EXPORT Q_DECL_IMPORT
# endif
#else
# define RCORE_EXPORT
#endif
