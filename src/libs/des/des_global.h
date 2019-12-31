#pragma once

#ifdef DES_EXPORTS
#define DLL_DES_API _declspec(dllexport)
#else
#define DLL_DES_API _declspec(dllimport)
#endif