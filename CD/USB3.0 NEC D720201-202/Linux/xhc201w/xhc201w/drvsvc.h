/*
	File Name : drvsvc.h
	
	Driver Service Control 

	* Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2009-09-29	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////


#ifndef _DRVSVC_H_
#define _DRVSVC_H_


//********************************************************************************//
//	define
//********************************************************************************//

// Service Unload Wait Time & Counter
#define SERVICE_DELETE_WAITSLEEP     100	// ms
#define SERVICE_DELETE_WAITCOUNT     10

//********************************************************************************//
//	struct
//********************************************************************************//

typedef struct _DISABLE_DEVICE{
	LIST_ENTRY	link;
	USHORT DevAddr;
}DISABLE_DEVICE,*PDISABLE_DEVICE;


//********************************************************************************//
//	Function
//********************************************************************************//

// Register/Unregister Service 
BOOL RegisterService(LPCTSTR drvPath);
BOOL UnregisterService(void);

// Open/Close Driver
HANDLE OpenDriver(void);
void CloseDriver(HANDLE hDrv);

// Enable/Disable Device
void InitDisableList(void);
int DisableDevice(USHORT devAddr, UINT nChild, DWORD *pError);
int EnableDevice(USHORT devAddr, DWORD *pError);
void EnableAllDevice(void);

#endif	// #ifndef _DRVSVC_H_
