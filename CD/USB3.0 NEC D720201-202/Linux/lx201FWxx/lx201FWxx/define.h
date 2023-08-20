/*
    File Name : define.h

    Standard define

    * Copyright (C) 2011 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//      History
//      2011-05-20  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>

// Definition of standard type
typedef char            TCHAR, LITERALCHAR, *PTCHAR;
typedef unsigned char   UCHAR;
typedef unsigned int    UINT;
typedef unsigned short  USHORT;
typedef unsigned long   ULONG, DWORD;
typedef unsigned int    DWORD_PTR;
typedef const char      *LPCSTR, *LPCWSTR, *LPCTSTR;
typedef int             errno_t;
typedef void            *LPVOID;
typedef bool            BOOL;
#define TRUE        true
#define FALSE       false

// Definition of macro
#define MAX_DIR    (256)
#define MAX_DRIVE  (3)
#define MAX_EXT    (256)
#define MAX_FNAME  (256)
#define MAX_PATH   (260)
#define LITERAL(a)       a

#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#define MAKEWPARAM(l, h)    ((WPARAM)(DWORD)MAKELONG(l, h))

// Wrapper of standard function
#define FPRINTF             fprintf
#define FPUTS               fputs
#define PRINTF              printf
#define STRLEN              strlen
#define STRTOUL             strtoul
#define STRCPY(a,b,c)     strcpy(a,c)
#define STRTOK(a,b,c)     strtok(a,b)
#define FOPEN_S(pFile,filename,mode)\
    ((*pFile = fopen(filename, mode)) != NULL)?0:(-1)
#define FCLOSE              fclose

inline int STRICMP(const char*str1, const char* str2) {
    while((*str1 != '\0')&&(*str2 != '\0')) {
        int c = (int)tolower(*str1) - (int)tolower(*str2);
        if(c != 0) {
            return c;
        }
        else {
            str1++;
            str2++;
        }
    }
    return (int)tolower(*str1) - (int)tolower(*str2);
}

// Wrapper of other function
#define COPYMEMORY          memcpy
#define SLEEP(a)            usleep(a * 1000)
#define ZEROMEMORY(dst,size)        memset(dst,0,size)

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

#define MAIN              main

#include "xhc201w.h"
#include "comdef.h"
#include "debug.h"

