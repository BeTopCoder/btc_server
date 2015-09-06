#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef _WIN32
    #ifdef EXPORT_DLL
        #define DLL_EXPORT __declspec(dllexport)
    #else
        #define DLL_EXPORT __declspec(dllimport) 
    #endif
#else
    #define DLL_EXPORT
#endif

#endif
