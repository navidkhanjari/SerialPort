/*
	File Name : log.cpp
	
	log control

	* Copyright (C) 2011 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2011-5-29	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////

#include "define.h"
#include "debug.h"


///////////////////////////////////////////////////////////////////////////////
//		Global
///////////////////////////////////////////////////////////////////////////////


FILE* g_LogFile = NULL;


///////////////////////////////////////////////////////////////////////////////
//		Common Function
///////////////////////////////////////////////////////////////////////////////

//
// Log file open
//
int OpenLogFile(LPCTSTR logname)
{
	errno_t err;

	if(g_LogFile){
		PRINTF (LITERAL("Other log file already opened!\n"));
		return W201STS_LOGFILE_ERR;
	}
	
	err = FOPEN_S (&g_LogFile, logname, LITERAL("a+"));

	if (err) {
		PRINTF (LITERAL("Log file open error: %s(a+) (%d)\n"), logname, err);
		return W201STS_LOGFILE_ERR;
	}
	
	return W201STS_SUCCESS;
}


//
// Log file close
//
void CloseLogFile(void)
{
	if(g_LogFile){
		FCLOSE (g_LogFile);
		g_LogFile = NULL;
	}
}


//
// Log write
//
int LogWrite(LPCTSTR command, USHORT devAddr, int status)
{

	if (g_LogFile == NULL) {
		return W201STS_LOGFILE_ERR;
	}

	if(devAddr != 0xFFFF){
		FPRINTF(g_LogFile,LITERAL("[%02X:%02X:%02X][Return = %d]"),
					GetBus(devAddr),GetDev(devAddr),GetFunc(devAddr),status);
	}
	else{
		FPRINTF(g_LogFile,LITERAL("[--:--:--][Return = %d]"),status);
	}
	
	if(FPUTS(command,g_LogFile) >= 0){
		return W201STS_LOGFILE_ERR;
	}


	return W201STS_SUCCESS;
	
}


