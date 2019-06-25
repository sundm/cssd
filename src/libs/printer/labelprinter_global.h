#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(LABELPRINTER_LIB)
#  define LABELPRINTER_EXPORT Q_DECL_EXPORT
# else
#  define LABELPRINTER_EXPORT Q_DECL_IMPORT
# endif
#else
# define LABELPRINTER_EXPORT
#endif


#define OK								0x00
#define INVALID_ERR						0x10
#define OPEN_TEMPLE_ERR					0x11
#define READ_TEMPLE_ERR					0x12
#define INIT_ENV_ERR					0x20
#define PRINTER_OPEN_ERR				0x21
#define PRINTER_INIT_ERR				0x22
#define PRINTER_START_DOC_ERR			0x23
#define PRINTER_START_PAGE_ERR			0x24
#define PRINTER_WRITE_DATA_ERR			0x25
#define PRINTER_END_PAGE_ERR			0x26
#define PRINTER_END_DOC_ERR				0x27
#define PRINTER_CHECK_ERR				0x28
#define PRINTER_CONVER_ERR				0x29