/*
	File Name : comutil.h
	
	Common Utility

	* Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2010-08-16  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _COMUTIL_H_
#define _COMUTIL_H_



//********************************************************************************//
//	Define
//********************************************************************************//

// Status Convert Entry
typedef struct _STATUS_CONV_ENTRY{
	DWORD	ErrSts;	// Error Code
	int		Xhcsts;	// XHC Status
}STATUS_CONV_ENTRY,*PSTATUS_CONV_ENTRY;

#ifdef _DEBUG
extern UINT g_MallocCount;
#endif

//********************************************************************************//
//	Function
//********************************************************************************//

// Convert Win32 Error Code to xhc status
int ConvertStatus(DWORD dwRet);


#ifdef _DEBUG

void* xhcmalloc(size_t size);
void xhcfree(void* ptr);

#else

#define xhcmalloc	malloc
#define xhcfree		free

#endif


#endif // #ifndef _COMUTIL_H_

