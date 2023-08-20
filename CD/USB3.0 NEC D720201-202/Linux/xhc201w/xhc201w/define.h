/*
    File Name : define.c

    Standard define

    * Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//      History
//      2010-08-16  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>


// Definition of standard type
typedef char                TCHAR, _TCHAR, *PTCHAR, *LPTSTR;
typedef unsigned char       UCHAR, BYTE, *PUCHAR;
typedef unsigned int        UINT;
typedef unsigned short      USHORT;
typedef unsigned long       ULONG, DWORD, *PULONG, *LPDWORD;
typedef unsigned int        DWORD_PTR;
typedef const char          *LPCSTR, *LPCWSTR, *LPCTSTR;
typedef int                 HANDLE;
typedef void                VOID;
typedef void                *LPVOID;
typedef int                 errno_t;
typedef bool                BOOL;
#define TRUE                true
#define FALSE               false
#define _stat               stat

#define CTL_CODE(a,b,c,d)   b
#define METHOD_BUFFERED
#define FILE_READ_DATA
#define FILE_WRITE_DATA
#define FILE_ANY_ACCESS

#include "intrface.h"


// Definition of list structure
typedef struct _list_entry {
    struct _list_entry      *Flink;
    struct _list_entry      *Blink;
} LIST_ENTRY, *PLIST_ENTRY;

// Wrapper of standard function
#define DECLARE_LASTERROR() int g_lastError
#define SetLastError(a)     g_lastError = a
#define GetLastError()      g_lastError
extern int g_lastError;

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#define COPYMEMORY          memcpy
#define FILLMEMORY(dst,size,data)   memset(dst,data,size)
#define ZEROMEMORY(dst,size)        memset(dst,0,size)
#define SLEEP(a)            usleep(a * 1000)
#define LITERAL(a)               a
#define FGETS               fgets
#define STRTOK              strtok_r
#define STRLEN              strlen
#define STRTOUL             strtoul
inline int STRICMP(const char*str1, const char* str2, int count) {
    while((*str1 != '\0')&&(*str2 != '\0')&&(count != 0)) {
        int c = (int)tolower(*str1) - (int)tolower(*str2);
        if(c != 0) {
            return c;
        }
        else {
            str1++;
            str2++;
            count--;
        }
    }
    if(count == 0) {
        return 0;
    }
    else {
        return (int)tolower(*str1) - (int)tolower(*str2);
    }
}

#define ISCNTRL           iscntrl
#define FOPEN(pFile,filename,mode)\
    ((*pFile = fopen(filename, mode)) != NULL)?0:ERROR_FILE_INVALID
#define FCLOSE              fclose
#define FILENO              fileno
#define FSTAT               fstat
#define FSEEK               fseek
#define FREAD               fread
#define FEOF                feof



#include "debug.h"      // debug
#include "intrface.h"   // driver interface
#include "comdef.h"     // common define
#include "err.h"     // error code define

inline BOOL CallDeviceIOControl(
    HANDLE          hDevice,            // Either of handle of device, file, and directory
    DWORD           dwIoControlCode,    // Control code of executed operation
    LPVOID          lpInBuffer,         // Pointer to buffer where input data is supplied
    DWORD           nInBufferSize,      // Size of each byte of input buffer
    LPVOID          lpOutBuffer,        // Pointer to buffer where output data is received
    DWORD           nOutBufferSize,     // Size of each byte of output buffer
    LPDWORD         lpBytesReturned,    // Pointer to variable that receives number of bytes for result
    void            *lpOverlapped       // Pointer to structure that shows asynchronization operation
)
{
    IOCTL_ARGP  argp;
    int         result;

    if(hDevice == XHCDRV_FILE_OPEN_ERR) {
        return FALSE;
    }

    argp.lpInBuffer         = lpInBuffer;
    argp.nInBufferSize      = nInBufferSize;
    argp.lpOutBuffer        = lpOutBuffer;
    argp.nOutBufferSize     = nOutBufferSize;

    result = ioctl((int)hDevice, dwIoControlCode, &argp);
    DebugTrace(LITERAL("CallDeviceIOControl(Device=%x,Command=%lx)\n"), hDevice, dwIoControlCode);

    if(result == 0) {
        *lpBytesReturned = argp.lpBytesReturned;
        return TRUE;
    }
    else {
        SetLastError(result);
        return FALSE;
    }
}


