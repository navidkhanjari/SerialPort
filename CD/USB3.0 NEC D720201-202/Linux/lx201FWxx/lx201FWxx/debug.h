/*
	File Name : debug.h
	
	Debug Output 

	* Copyright (C) 2011 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2011-05-20	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _W201FW_DEBUG_H_
#define _W201FW_DEBUG_H_

#ifdef _DEBUG
#define DebugTrace(...)     printf(__VA_ARGS__)

// As for gcc, it makes an error of the description called (__FUNCTION__ "xxx=%d")
//  because __FUNCTION__ is not treated as a string literal.
// Therefore, the macro that involves __FUNCTION__ is defined.
#define DebugTraceFunction(...)\
    printf("%s",__FUNCTION__);\
    printf(__VA_ARGS__)
#else
#define DebugTrace(...)
#define DebugTraceFunction(...)
#endif

#ifdef _DEBUG
#define DebugPause()		_gettchar()
#else
#define DebugPause()
#endif



#endif //#ifndef _W201FW_DEBUG_H_
