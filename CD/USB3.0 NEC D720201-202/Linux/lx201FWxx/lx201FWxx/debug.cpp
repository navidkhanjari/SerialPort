/*
	File Name : debug.cpp
	
	Debug Output 

	* Copyright (C) 2011 Renesas Electronics Corporation
*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2011-05-20	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////


#include "define.h"

#include "debug.h"

//********************************************************************************//
//	Global
//********************************************************************************//
extern TCHAR g_AppName[9];

//********************************************************************************//
//	Function
//********************************************************************************//

#ifdef _DEBUG

void DebugTrace( LPCTSTR pszFormat, ...)
{
	va_list	args;

    va_start(args, pszFormat);
    vprintf(pszFormat,args);

    va_end(args);
}


#endif	// #ifdef _DEBUG


