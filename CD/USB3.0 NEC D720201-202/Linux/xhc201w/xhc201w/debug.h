/*
	File Name : debug.h
	
	Debug Output 

	* Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2010-08-16  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _XHCDLL_DEBUG_H_
#define _XHCDLL_DEBUG_H_

#ifdef _DEBUG
void DebugTrace( LPCTSTR pszFormat, ...);
#else
#define DebugTrace(...)
#endif

#define _XHCI_D720201

#define DebugTraceFunction(...)\
        DebugTrace("%s", __FUNCTION__);\
        DebugTrace(__VA_ARGS__)

#endif //#ifndef _XHCDLL_DEBUG_H_
