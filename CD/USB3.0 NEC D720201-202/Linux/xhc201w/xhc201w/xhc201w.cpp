/*
    File Name : xhc201w.c

    Library for access to xhc201 driver

    * Copyright (C) 2011 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//      History
//      2011-05-20  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////

#include "define.h"
#include "xhc201w.h"
#include "xhc201h.h"

#include "comutil.h"	// common utility

#include "devio.h"
#include "sromio.h"



//********************************************************************************//
//	Global
//********************************************************************************//
HANDLE	g_hDriver = XHCDRV_FILE_OPEN_ERR;			// Driver Handle

static unsigned short	g_DevAddr = 0xFFFF;	// Current Open Device
static unsigned short	g_OpMode = -1;		// Current Open Mode
XHC_SROM_INFO	g_SROMInfo;			// Current SROM Information
BOOL			g_bSROMInfo;		// SROM Information set flag

// Global Result Code(LastErrot())
DECLARE_LASTERROR();


//********************************************************************************//
//	Global (extern)
//********************************************************************************//



//********************************************************************************//
//	API
//********************************************************************************//

/*
	Common Function
*/

// Library Initialize
int _API xHcInitialize(void)
{
	int ret = XHCSTS_SUCCESS;

	DebugTraceFunction(LITERAL(" ++\n"));

	g_hDriver = (HANDLE)open(XHCDRV_FILE_NAME, O_RDWR);
	if(g_hDriver == XHCDRV_FILE_OPEN_ERR){
		// error
		ret = XHCSTS_DRVLOAD_ERROR;
	}

	g_bSROMInfo = FALSE;

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);


	return ret;
}


// Library Terminate
int  _API xHcTerminate(void)
{
	int ret = XHCSTS_SUCCESS;

	DebugTraceFunction(LITERAL(" ++\n"));

	if(g_hDriver != XHCDRV_FILE_OPEN_ERR){
		close((int)g_hDriver);
	}
	g_hDriver = XHCDRV_FILE_OPEN_ERR;

	g_bSROMInfo = FALSE;

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);


	return ret;
}


// Search Target Device
int _API xHcFindDevice(
					unsigned short venID,
					unsigned short devID,
					PXHC_PCIDEV_INFO pDevInfo,
					int num,
					unsigned int *pFindCount
					)
{
	PXHCUTIL_FINDDEVICE pFind = NULL;
	DWORD dwSize;
	int i,count;
	int ret;
	BOOL bRet;

	DebugTraceFunction(LITERAL(" ++\n"));

	// driver check;
	if(!g_hDriver){
		ret = XHCSTS_NOT_INITIALIZE;
		goto Exit_xHcFindDevice;
	}

	// parameter check
	if(num < 0){
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcFindDevice;
	}
	else if((num > 0) && (!pDevInfo)){
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcFindDevice;
	}
	

	// Buffer allocate
	pFind = (PXHCUTIL_FINDDEVICE)malloc(sizeof_XHCUTIL_FINDDEVICE(num));
	
	if(!pFind){
		ret = XHCSTS_MEMORY_ERROR;
		goto Exit_xHcFindDevice;
	}

	// parameter setting
	if(venID){
		pFind->VendorID = venID;
	}
	else{
		pFind->VendorID = XHC201_VENDOR_ID;
	}

	if(devID){
		pFind->DeviceID = devID;
	}
	else{
		pFind->DeviceID = XHC201_DEVICE_ID;
	}
	
	pFind->nBufCount = num;
	pFind->nDevice = 0;

	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_FIND_DEVICE,
					pFind,
					sizeof_XHCUTIL_FINDDEVICE(0),
					pFind,
					sizeof_XHCUTIL_FINDDEVICE(num),
					&dwSize,
					NULL
					);

	if(!bRet){
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_FIND_DEVICE] fail (%d)\n"),GetLastError());
		ret = XHCSTS_FIND_DEVICE_ERROR;
		goto Exit_xHcFindDevice;
	}
	
	count = min((unsigned int)num,pFind->nDevice);

	// Result Copy
	for(i=0;i<count;i++){
		pDevInfo[i].DevAddr = GET_PCIADDR(pFind->DevInfo[i].DevAddr);
		pDevInfo[i].SubSystemVendorID = pFind->DevInfo[i].SuvSystemVendorID;
		pDevInfo[i].SubSystemID = pFind->DevInfo[i].SuvSystemID;
		pDevInfo[i].RevisionID = pFind->DevInfo[i].RevisionID;

		DebugTrace(LITERAL("%d : Device[%02X:%02X:%02X] Revision (%d)\n"),
							i, GetBus(pDevInfo[i].DevAddr), GetDev(pDevInfo[i].DevAddr),
							GetFunc(pDevInfo[i].DevAddr), pDevInfo[i].RevisionID);
		DebugTrace(LITERAL("\tSubSystemVendorID = 0x%04X , SubSystemID = 0x%04X\n"),
							pDevInfo[i].SubSystemVendorID, pDevInfo[i].SubSystemID);

	}

	if(pFindCount){
		*pFindCount = pFind->nDevice;
	}

	ret = XHCSTS_SUCCESS;


Exit_xHcFindDevice:

	if(pFind){
		free(pFind);
	}

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}


// Open Target Device
int _API xHcOpenDevice(
					unsigned short devAddr,
					unsigned int mode,
					PXHC_SROM_INFO pSROMInfo,
					ULONG targetRevID
					)
{
	unsigned long addr;
	DWORD dwSize;
	DWORD dwErr;
	int ret;
	BOOL bRet;


	DebugTraceFunction(LITERAL(" ++\n"));

	// driver check;
	if(!g_hDriver){
		ret = XHCSTS_NOT_INITIALIZE;
		goto Exit_xHcOpenDevice;
	}

	// Already opened
	if(g_DevAddr != 0xFFFF){
		ret = XHCSTS_ALREADY_OPEN;
		goto Exit_xHcOpenDevice;
	}


	// parameter check
	if(((mode & XHC_OPMODE_NORMAL) == 0) &&
	   ((mode & XHC_OPMODE_UFW) == 0))
	{
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcOpenDevice;
	}

	// parameter check
	if((mode & XHC_OPMODE_NORMAL) &&
	   (mode & XHC_OPMODE_UFW))
	{
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcOpenDevice;
	}

	// Open
	addr = MAKE_PCIADDR(devAddr);

	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_DEVICE_OPEN,
					&addr,
					sizeof(unsigned long),
					NULL,
					0,
					&dwSize,
					NULL
					);

	if(!bRet){
		dwErr = GetLastError();

		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_DEVICE_OPEN] fail (%d)\n"),dwErr);
		ret = ConvertStatus(dwErr);

		goto Exit_xHcOpenDevice;
	}

	g_DevAddr = devAddr;
	g_OpMode = mode;

	if(pSROMInfo){
		// copy
		COPYMEMORY(&g_SROMInfo,pSROMInfo,sizeof(XHC_SROM_INFO));
		g_bSROMInfo = TRUE;
	}
	else{
		// default set
		g_SROMInfo.Size = XHC201_SROM_SIZE;
		g_SROMInfo.Page = XHC201_SROM_PAGE;
		g_SROMInfo.Code = XHC201_SROM_CODE;
	}

	ret = XHCSTS_SUCCESS;


Exit_xHcOpenDevice:

	if(ret != XHCSTS_SUCCESS){

		if(g_DevAddr != 0xFFFF){
			xHcCloseDevice(devAddr);
		}

		g_bSROMInfo = FALSE;

	}

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}


// Close Target Device
int _API xHcCloseDevice(unsigned short devAddr)
{
	unsigned long addr;
	DWORD dwSize;
	DWORD dwErr;
	int ret;
	BOOL bRet;

	DebugTraceFunction(LITERAL(" ++\n"));

	// driver check;
	if(!g_hDriver){
		ret = XHCSTS_NOT_INITIALIZE;
		goto Exit_xHcCloseDevice;
	}

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcCloseDevice;
	}


	// Devcice Reset
	if(g_OpMode == XHC_OPMODE_UFW){
//		ret = xHcResetDevice(devAddr);

		// result is no check
	}


	// Close
	addr = MAKE_PCIADDR(devAddr);

	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_DEVICE_CLOSE,
					&addr,
					sizeof(unsigned long),
					NULL,
					0,
					&dwSize,
					NULL
					);

	if(!bRet){
		dwErr = GetLastError();
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_DEVICE_CLOSE] fail (%d)\n"),dwErr);

		// result is no check
	}

	// Clear device address
	g_DevAddr = 0xFFFF;

	g_bSROMInfo = FALSE;


Exit_xHcCloseDevice:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}


// Reset Target Device
int _API xHcResetDevice(unsigned short devAddr)
{
	unsigned long addr;
	DWORD dwSize;
	int ret;
	BOOL bRet;

	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcResetDevice;
	}


	// Open mode check
	if(g_OpMode != XHC_OPMODE_UFW){
		ret = XHCSTS_INVALID_REQUEST;
		goto Exit_xHcResetDevice;
	}


	// Close
	addr = MAKE_PCIADDR(devAddr);

	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_DEVICE_RESET,
					&addr,
					sizeof(unsigned long),
					NULL,
					0,
					&dwSize,
					NULL
					);

	if(!bRet){
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_DEVICE_RESET] fail (%d)\n"),GetLastError());
		ret = XHCSTS_DEVRESET_ERROR;

		goto Exit_xHcResetDevice;
	}

	g_OpMode = XHC_OPMODE_NORMAL;

	ret = XHCSTS_SUCCESS;

Exit_xHcResetDevice:
	
	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);
	return ret;
}



/*
	Serial ROM Function
*/

// Auto Detect Serial ROM
int _API xHcDetectSerialROM(
					unsigned short devAddr,
					PXHC_SROM_INFO pSROMInfo
					)
{
	XHC_DEVICE_INFO devInfo;
	int ret;

	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcDetectSerialROM;
	}


	// Open mode check
	if(g_OpMode != XHC_OPMODE_UFW){
		ret = XHCSTS_INVALID_REQUEST;
		goto Exit_xHcDetectSerialROM;
	}

	// parameter check
	if(!pSROMInfo){
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcDetectSerialROM;
	}

	ret = GetSROMIdentify(devAddr,pSROMInfo,TRUE);

	if(ret == XHCSTS_UNKNOWN_SROM){
		ret = GetDeviceInfo(devAddr,&devInfo);

		if(ret != XHCSTS_SUCCESS){
			goto Exit_xHcDetectSerialROM;
		}

		if(devInfo.SROMInfo.Size){
			COPYMEMORY(pSROMInfo, &devInfo.SROMInfo, sizeof(XHC_SROM_INFO));
		}
		else{
			ret = XHCSTS_UNKNOWN_SROM;
		}
	}

	if(ret == XHCSTS_SUCCESS){
		COPYMEMORY(&g_SROMInfo, pSROMInfo, sizeof(XHC_SROM_INFO));
		g_bSROMInfo = TRUE;

		DebugTrace(LITERAL("SROM Size = 0x%08X\n"),pSROMInfo->Size);
		DebugTrace(LITERAL("SROM Page = 0x%04X\n"),pSROMInfo->Page);
		DebugTrace(LITERAL("SROM Code = 0x%04X\n"),pSROMInfo->Code);
	}


Exit_xHcDetectSerialROM:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}


// Set Serial ROM Information
int _API xHcSetSerialROMInfo(
					unsigned short devAddr,
					PXHC_SROM_INFO pSROMInfo
					)
{
	int ret;

	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcSetSerialROMInfo;
	}


	// Open mode check
	if(g_OpMode != XHC_OPMODE_UFW){
		ret = XHCSTS_INVALID_REQUEST;
		goto Exit_xHcSetSerialROMInfo;
	}

	// parameter check
	if(!pSROMInfo){
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcSetSerialROMInfo;
	}

	ret = SetSROMIdentify(devAddr, pSROMInfo);
	g_bSROMInfo = TRUE;

	DebugTrace(LITERAL("SROM Parameter = 0x%08lX\n"),pSROMInfo->Code);

Exit_xHcSetSerialROMInfo:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}


// Get Device Information
int _API xHcGetDeviceInfo(
					unsigned short devAddr,
					PXHC_DEVICE_INFO pDevInfo
					)
{
	int ret;

	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcGetDeviceInfo;
	}


	// Open mode check
	if(g_OpMode != XHC_OPMODE_UFW){
		ret = XHCSTS_INVALID_REQUEST;
		goto Exit_xHcGetDeviceInfo;
	}	

	// parameter check
	if(!pDevInfo){
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcGetDeviceInfo;
	}


	ret = GetDeviceInfo(devAddr,pDevInfo);

Exit_xHcGetDeviceInfo:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}



// Read Serial ROM
int _API xHcReadSerialROM(
					unsigned short devAddr,
					unsigned int offset,
					unsigned long size,
					unsigned char *pReadData,
					unsigned long *pReadReturned,
					int comp
					)
{
	int ret;
	
	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcReadSerialROM;
	}

	// Read SROM
	if(comp == XHC_WAIT_COMPLETE){

		if(!pReadData){
			ret = XHCSTS_PARAMETER_ERROR;
			goto Exit_xHcReadSerialROM;

		}

		ret = ReadSROM( devAddr,
						offset,
						size,
						pReadData,
						pReadReturned,
						&g_SROMInfo);

	}
	else{

		ret = ReadSROM( devAddr,
						offset,
						size,
						NULL,
						NULL,
						&g_SROMInfo);
	}


Exit_xHcReadSerialROM:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);
	return ret;
}

// Get Read Data
int _API xHcGetReadData(
					unsigned short devAddr,
					unsigned long size,
					unsigned char *pReadData,
					unsigned long *pReadReturned
					)
{
	int ret;

	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcGetReadData;
	}

	// Open mode check
	if(g_OpMode != XHC_OPMODE_UFW){
		ret = XHCSTS_INVALID_REQUEST;
		goto Exit_xHcGetReadData;
	}	

	// parameter check
	if(!pReadData){
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcGetReadData;
	}

	// Read Data
	ret = GetReadData(devAddr, size, pReadData, pReadReturned);

Exit_xHcGetReadData:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);
	return ret;
}

// Write Serial ROM Image
int _API xHcWriteSerialROM(
					unsigned short devAddr,
					unsigned long size,
					unsigned char *pWriteData,
					int comp
					)
{
	int ret;
	
	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcWriteSerialROM;
	}


	// Open mode check
	if(g_OpMode != XHC_OPMODE_UFW){
		ret = XHCSTS_INVALID_REQUEST;
		goto Exit_xHcWriteSerialROM;
	}	

	// parameter check
	if((comp != XHC_WAIT_COMPLETE) &&
	   (comp != XHC_NO_WAIT))
	{
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcWriteSerialROM;
	}

	if(!size){
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcWriteSerialROM;
	}

	if(!g_bSROMInfo){
		ret = XHCSTS_UNKNOWN_SROM;
		goto Exit_xHcWriteSerialROM;
	}

	// Write SROM
	if(comp == XHC_WAIT_COMPLETE){

		ret = WriteSROM(devAddr, 0, size, pWriteData, NULL, TRUE);

	}
	else{

		ret = WriteSROM(devAddr, 0, size, pWriteData, NULL, FALSE);
	}


Exit_xHcWriteSerialROM:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);
	return ret;
}


// Erase Serial ROM
int _API xHcEraseSerialROM(
					unsigned short devAddr,
					int comp
					)
{
	int ret;
	
	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcEraseSerialROM;
	}


	// Open mode check
	if(g_OpMode != XHC_OPMODE_UFW){
		ret = XHCSTS_INVALID_REQUEST;
		goto Exit_xHcEraseSerialROM;
	}	

	// parameter check
	if((comp != XHC_WAIT_COMPLETE) &&
	   (comp != XHC_NO_WAIT))
	{
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcEraseSerialROM;
	}

	if(!g_bSROMInfo){
		ret = XHCSTS_UNKNOWN_SROM;
		goto Exit_xHcEraseSerialROM;
	}

	// Erase SROM
	if(comp == XHC_WAIT_COMPLETE){

		ret = EraseSROM(devAddr, NULL, TRUE);

	}
	else{

		ret = EraseSROM(devAddr, NULL, FALSE);
	}


Exit_xHcEraseSerialROM:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);
	return ret;

}


// Get Write/Erase Status
int _API xHcGetWriteStatus(
					unsigned short devAddr,
					unsigned long *pWriteSize
					)
{
	int ret;

	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcGetWriteStatus;
	}


	// Open mode check
	if(g_OpMode != XHC_OPMODE_UFW){
		ret = XHCSTS_INVALID_REQUEST;
		goto Exit_xHcGetWriteStatus;
	}	

	// Get Status
	ret = GetWriteStatus(devAddr, pWriteSize);

Exit_xHcGetWriteStatus:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);
	return ret;
}

// Set ExtROM Access Enable
int _API xHcSetExtROMAccessEnable(
					unsigned short devAddr,
					unsigned short en
					)
{
	int ret;

	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcSetExtROMAccessEnable;
	}

	// Open mode check
	if(g_OpMode != XHC_OPMODE_UFW){
		ret = XHCSTS_INVALID_REQUEST;
		goto Exit_xHcSetExtROMAccessEnable;
	}

	// Set ExtROMAccessEnable
	ret = SetExtROMAccessEnable(devAddr, en);

Exit_xHcSetExtROMAccessEnable:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);
	return ret;
}

// Get SROM Write Result code
int _API xHcGetROMWriteResultCode(
					unsigned short devAddr,
					unsigned long* pResultCode
					)
{
	int ret;

	DebugTraceFunction(LITERAL(" ++\n"));

	//  open check
	if(g_DevAddr != devAddr){
		ret = XHCSTS_NOT_OPENED;
		goto Exit_xHcGetROMWriteResultCode;
	}

	// Open mode check
	if(g_OpMode != XHC_OPMODE_UFW){
		ret = XHCSTS_INVALID_REQUEST;
		goto Exit_xHcGetROMWriteResultCode;
	}

	// Get SROM Write ResultCode
	ret = GetROMWriteResultCode(devAddr, pResultCode);

Exit_xHcGetROMWriteResultCode:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);
	return ret;
}

// Update Serial ROM Image from file(CORE)
int xHcUpdateROMImageFromFile(
					LPCTSTR lpMemPath,
					LPCTSTR lpCfgPath,
					LPCTSTR lpEui64Path,
					PXHC_SROM_INFO pSROMInfo,
					unsigned char* pImageData
					)
{

	int ret;

	DebugTraceFunction(LITERAL(" ++\n"));

	// parameter check
	if((!lpMemPath) || (!pImageData)){
		ret = XHCSTS_PARAMETER_ERROR;
		goto Exit_xHcUpdateROMImageFromFile;
	}

	// Make write image
	ret = UpdateSROMImageFromFile(
					lpMemPath,
					lpCfgPath,
					lpEui64Path,
					pSROMInfo,
					pImageData
					);


	if(ret != XHCSTS_SUCCESS){
		DebugTrace(LITERAL("Update SROM image fail(%d)\n"),ret);
		goto Exit_xHcUpdateROMImageFromFile;
	}


Exit_xHcUpdateROMImageFromFile:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}

