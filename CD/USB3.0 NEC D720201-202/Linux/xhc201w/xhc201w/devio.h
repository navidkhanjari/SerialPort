/*
	File Name : devio.h
	
	Device I/O 

	* Copyright (C) 2010 Renesas Electronics Corporation


*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2009-10-10	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DEVICEIO_H_
#define _DEVICEIO_H_

//********************************************************************************//
//	define
//********************************************************************************//

// UFW load wait
#define XHC_RESET_WAIT_COUNT	500	// times
#define XHC_RESET_WAIT			10	// ms

//********************************************************************************//
//	Function
//********************************************************************************//

// Read Write PCI Memory/Register
BOOL ReadPCI(PXHCUTIL_ADDRESS pAddr);
BOOL WritePCI(PXHCUTIL_ADDRESS pAddr);

BOOL ReadAHB(PXHCUTIL_ADDRESS pAddr);
BOOL WriteAHB(PXHCUTIL_ADDRESS pAddr);
BOOL LoadUtilFw(USHORT devAddr, ULONG targetRevID);



#endif // #ifndef _DEVICEIO_H_

