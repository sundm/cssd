#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(RDAO_LIB)
#  define RDAO_EXPORT Q_DECL_EXPORT
# else
#  define RDAO_EXPORT Q_DECL_IMPORT
# endif
#else
# define RDAO_EXPORT
#endif
