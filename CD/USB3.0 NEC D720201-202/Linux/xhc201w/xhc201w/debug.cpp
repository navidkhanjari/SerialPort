/*
	File Name : debug.c
	
	Debug Output 

	* Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//      2010-08-16  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////


#include "define.h"

#include "debug.h"

//********************************************************************************//
//	Global
//********************************************************************************//


//********************************************************************************//
//	Function
//********************************************************************************//

#ifdef _DEBUG

void DebugTrace( LPCTSTR pszFormat, ...)
{
	va_list	args;

    printf("[%s]", MODULE_NAME);

    va_start(args, pszFormat);
    vprintf(pszFormat, args);
    va_end(args);

}


#endif	// #ifdef _DEBUG


