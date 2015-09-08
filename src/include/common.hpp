#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#pragma warning(disable: 4251)
#pragma warning(disable: 4273)

#ifdef _WIN32
    #ifdef EXPORT_DLL
        #define DLL_EXPORT __declspec(dllexport)
    #else
        #define DLL_EXPORT __declspec(dllimport) 
    #endif
#else
    #define DLL_EXPORT
#endif

typedef char                CHAR;
typedef unsigned char       UCHAR;
typedef signed char         LONG_8;
typedef unsigned char       ULONG_8;
typedef signed short        LONG_16;
typedef unsigned short      ULONG_16;
typedef signed int          LONG_32;
typedef unsigned int        ULONG_32;
typedef signed long long    LONG_64;
typedef unsigned long long  ULONG_64;
typedef char                BYTE;

#define NAMESPACE_BEGIN(n)      namespace n {
#define NAMESPACE_END(n)        }


#endif
