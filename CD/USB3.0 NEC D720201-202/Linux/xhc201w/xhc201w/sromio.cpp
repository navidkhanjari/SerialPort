/*
	File Name : sromio.c
	
	Serial ROM I/O 

	* Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//      2010-08-16  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////


#include "define.h"
#include "xhc201w.h"
#include "xhc201h.h"

#include "comutil.h"	// common utility
#include "xhcutlfw.h"

#include "fileio.h"

#include "sromio.h"

//********************************************************************************//
//	Global
//********************************************************************************//
extern HANDLE g_hDriver;			// Driver Handle (xhc201w.cpp)
extern XHC_SROM_INFO g_SROMInfo;	// Current SROM Information
extern BOOL	 g_bSROMInfo;			// SROM Information set flag

/* CRC16 Table */
const USHORT crc_table[16] = {
    0x0000, 0x1081, 0x2102, 0x3183, 0x4204, 0x5285, 0x6306, 0x7387,
    0x8408, 0x9489, 0xA50A, 0xB58B, 0xC60C, 0xD68D, 0xE70E, 0xF78F
};

//********************************************************************************//
//	define
//********************************************************************************//

// Utility FW Result 
#define UFW_UNKNOWN_SROM		((DWORD)XHCUTLFW_SR_UNKNOWN_SERIALROM << 16)	// Unknown Serial ROM

#define SET_VSCB_DATA(pbuf, offset, data, addr, size)														\
{																											\
	int setLoop;																							\
	int bufset;																								\
	bufset = offset;																						\
	for(setLoop = 0; setLoop < size; setLoop++){															\
		pbuf[bufset++] = (unsigned char)(addr+setLoop);														\
		pbuf[bufset++] = (unsigned char)((data >> (VSC_DATA_SHIFT_BIT*setLoop)) & VSC_DATA_BLOCK_MASK);		\
	}																										\
}

//********************************************************************************//
//	Function
//********************************************************************************//

//********************************************************************************//
//	Extern
//********************************************************************************//
extern int g_findDevFlag;

//--------------------------------------------------------------------------------
//	SROM Image Access Function
//--------------------------------------------------------------------------------

//
// Get SROM size 
//
ULONG GetSROMSize(PXHC_SROM_INFO pSROMInfo, BOOL bDevice)
{
	ULONG romSize;

	if(pSROMInfo){
		if(pSROMInfo->Size){
			 romSize = min(MAX_SROM_SIZE, pSROMInfo->Size);
		}
		else{
			romSize = MAX_SROM_SIZE;
		}
	}
	else if((bDevice) && (g_bSROMInfo)){
	
		if(g_SROMInfo.Size){
			romSize = min(MAX_SROM_SIZE, g_SROMInfo.Size);
		}
		else{
			romSize = MAX_SROM_SIZE;
		}
	}
	else{
		romSize = MAX_SROM_SIZE;
	}

	return romSize;
}

//
//	Update SROM Image from File
//
int UpdateSROMImageFromFile(
			LPCTSTR lpMemPath,
			LPCTSTR lpCfgPath,
			LPCTSTR lpEui64Path,
			PXHC_SROM_INFO pSROMInfo,
			UCHAR* pImageData
		   )
{

	UCHAR* pTemp = NULL;
    ULONG size,fsize;
    USHORT subsysID,subvenID;
    UCHAR eui64[8];
	XHC_CFG_SETTING_INFO configSetting;
	int buf_vsc_offset = 0;
	int loop;
	UCHAR vsbAddr;
	USHORT crcResult;
	ULONG romSize;
	int ret;
	BOOL footer = FALSE;

	romSize = GetSROMSize(pSROMInfo, FALSE);

	pTemp = (UCHAR*)xhcmalloc(romSize);

	if(!pTemp){
		DebugTrace(LITERAL("Memory allocate fail(%d)\n"),romSize);
		ret = XHCSTS_MEMORY_ERROR;
		goto Exit_UpdateSROMImageFromFile;
	}

	// Initialize
	configSetting.phySetting1 = 0;
	configSetting.phySetting2 = 0;
	if(g_findDevFlag == DEVICE_ID_TYPE_D201){
		configSetting.phySetting3 = CFG_D201_DEFAULT_VALUE_E4;
		configSetting.chipSetting = CFG_D201_DEFAULT_VALUE_E8;
	}
	else if(g_findDevFlag == DEVICE_ID_TYPE_D202){
		configSetting.phySetting3 = CFG_D202_DEFAULT_VALUE_E4;
		configSetting.chipSetting = CFG_D202_DEFAULT_VALUE_E8;
	}
	else{
		configSetting.phySetting3 = CFG_D201_DEFAULT_VALUE_E4;
		configSetting.chipSetting = CFG_D201_DEFAULT_VALUE_E8;
	}

	if(lpCfgPath){
		ret = ReadVendorSpecificConfigInfo(lpCfgPath,
										   &subsysID,
										   &subvenID,
										   &configSetting,
										   &footer);
		if(ret != XHCSTS_SUCCESS){
			goto Exit_UpdateSROMImageFromFile;
		}

		DebugTrace(LITERAL("Load Config file:: subsysvenID = 0x%04X , subsysID = 0x%04X\n"), subvenID,subsysID);
		DebugTrace(LITERAL("Load Config file:: PHY Settings1 = 0x%08lX\n"), configSetting.phySetting1);
		DebugTrace(LITERAL("Load Config file:: PHY Settings2 = 0x%08lX\n"), configSetting.phySetting2);
		DebugTrace(LITERAL("Load Config file:: PHY Settings3 = 0x%08lX\n"), configSetting.phySetting3);
		DebugTrace(LITERAL("Load Config file:: Chip Settings = 0x%08lX\n"), configSetting.chipSetting);

// A leader of Vendor Specific Configuration Block sets Length (2Byte)
		buf_vsc_offset = VSC_DATA_BLOCK_SIZE_2;

// Generation of Vendor Specific Configuration Block
// Upper byte : DATA, Lowerbyte: Address
// SubSystem Vendor ID -> Addr: 01h - 00h
		SET_VSCB_DATA(pTemp, buf_vsc_offset, subvenID, VSC_DATA_ADDR_SUBSYSTEM_VID, VSC_DATA_BLOCK_SIZE_2);
		buf_vsc_offset += (VSC_DATA_BLOCK_SIZE_2*2);	// 1 block size is 2Byte.

// SubSystem ID -> Addr: 03h - 02h
		SET_VSCB_DATA(pTemp, buf_vsc_offset, subsysID, VSC_DATA_ADDR_SUBSYSTEM_ID, VSC_DATA_BLOCK_SIZE_2);
		buf_vsc_offset += (VSC_DATA_BLOCK_SIZE_2*2);
	}

// EUI-64 file reading
	if(lpEui64Path){
		ret = ReadEui64(lpEui64Path,eui64,1);

		if(ret != XHCSTS_SUCCESS){
			goto Exit_UpdateSROMImageFromFile;
		}

		DebugTrace(LITERAL("EUI-64 : %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n"),
								eui64[0],eui64[1],eui64[2],eui64[3],
								eui64[4],eui64[5],eui64[6],eui64[7]);

// Generation of Vendor Specific Configuration Block
// Serial Number 1st -> Addr: 07h - 04h
// Serial Number 2nd -> Addr: 0Bh - 08h
		vsbAddr = VSC_DATA_ADDR_SERIAL_NUM1;
		for(loop = 0; loop < VSC_DATA_BLOCK_SIZE_8; loop++)
		{
			SET_VSCB_DATA(pTemp, buf_vsc_offset, eui64[loop], vsbAddr, VSC_DATA_BLOCK_SIZE_1);
			buf_vsc_offset += (VSC_DATA_BLOCK_SIZE_1*2);
			vsbAddr++;
		}
	}

	if(lpCfgPath){
// Generation of Vendor Specific Configuration Block
		SET_VSCB_DATA(pTemp, buf_vsc_offset, configSetting.phySetting1, VSC_DATA_ADDR_PHY_SETTING1, VSC_DATA_BLOCK_SIZE_4);
		buf_vsc_offset += (VSC_DATA_BLOCK_SIZE_4*2);

		SET_VSCB_DATA(pTemp, buf_vsc_offset, configSetting.phySetting2, VSC_DATA_ADDR_PHY_SETTING2, VSC_DATA_BLOCK_SIZE_4);
		buf_vsc_offset += (VSC_DATA_BLOCK_SIZE_4*2);

		SET_VSCB_DATA(pTemp, buf_vsc_offset, configSetting.phySetting3, VSC_DATA_ADDR_PHY_SETTING3, VSC_DATA_BLOCK_SIZE_4);
		buf_vsc_offset += (VSC_DATA_BLOCK_SIZE_4*2);

		SET_VSCB_DATA(pTemp, buf_vsc_offset, configSetting.chipSetting, VSC_DATA_ADDR_CHIP_SETTING, VSC_DATA_BLOCK_SIZE_4);
		buf_vsc_offset += (VSC_DATA_BLOCK_SIZE_4*2);
	}


	if(buf_vsc_offset != 0){
// Length (2Byte) is set in the head of Vendor Specific Configuration Block.
		pTemp[0] = (unsigned char)((buf_vsc_offset-VSC_DATA_BLOCK_SIZE_2) & VSC_DATA_BLOCK_MASK);
		pTemp[1] = (unsigned char)(((buf_vsc_offset-VSC_DATA_BLOCK_SIZE_2) >> VSC_DATA_SHIFT_BIT) & VSC_DATA_BLOCK_MASK);
// Calculation of CRC16
		crcResult = update_crc16(CRC_16, &pTemp[0], buf_vsc_offset);
		pTemp[buf_vsc_offset]   = (unsigned char)(crcResult & VSC_DATA_BLOCK_MASK);
		pTemp[buf_vsc_offset+1] = (unsigned char)((crcResult >> VSC_DATA_SHIFT_BIT) & VSC_DATA_BLOCK_MASK);
		buf_vsc_offset += VSC_DATA_BLOCK_SIZE_2;
	}

// SROM  Generation of an image file.
// image file open
	ret = LoadSROMImage(lpMemPath, romSize, &pTemp[buf_vsc_offset], &fsize, &size);

	if(ret != XHCSTS_SUCCESS){
		goto Exit_UpdateSROMImageFromFile;
	}

	if((fsize + buf_vsc_offset) > romSize){
		DebugTrace(LITERAL("ROM Image file size over: %s\n"), lpMemPath);
		ret = XHCSTS_MEMORY_ERROR;
		goto Exit_UpdateSROMImageFromFile;
	}

	COPYMEMORY(pImageData, pTemp, (size+buf_vsc_offset));
	FILLMEMORY(&pImageData[size+buf_vsc_offset], (pSROMInfo->Size - (size+buf_vsc_offset)), 0xFF);

	if(footer == TRUE){
		*((unsigned short*)&pImageData[(size_t)(FW_UPDATER_FOOTER_SIZE - 2)]) = (unsigned short)SROM_END_CODE;
	}

Exit_UpdateSROMImageFromFile:

	if(pTemp){
		xhcfree(pTemp);
	}

	return ret;
}


//--------------------------------------------------------------------------------
//	Utility Function
//--------------------------------------------------------------------------------

//
// Make SROM access request
//
PXHCUTIL_SROMACC MakeSROMRequest(USHORT devAddr,
								 ULONG offset,
								 ULONG size,
								 PXHC_SROM_INFO pSROMInfo,
								 BOOL bData
								 )
{

	PXHCUTIL_SROMACC pAcc;
	int length;
	
	// data buffer
	if(bData){
		length = sizeof_XHCUTIL_SROMACC(size);
	}
	// no data buffer
	else{
		length = sizeof_XHCUTIL_SROMACC(0);
	}


	pAcc = (PXHCUTIL_SROMACC)xhcmalloc(length);


	if(!pAcc){
		DebugTrace(LITERAL("Memory allocate fail(%d)\n"),length);
		return NULL;
	}

	pAcc->DevAddr = MAKE_PCIADDR(devAddr);
	pAcc->Offset = offset;
	pAcc->Size = size;
	
	if(pSROMInfo){
		pAcc->PageSize = pSROMInfo->Page;
		pAcc->DevCode = pSROMInfo->Code;
	}
	else{
		pAcc->PageSize = 0;
		pAcc->DevCode = 0;
	}

	DebugTrace(LITERAL("Allocate Serial Rom Request (0x%p)\n"),pAcc);

	return pAcc;
}


//
// Release SROM access request
//
void FreeSROMRequest(PXHCUTIL_SROMACC pAcc)
{
	DebugTrace(LITERAL("Release Serial Rom Request (0x%p)\n"),pAcc);

	xhcfree(pAcc);
}
//--------------------------------------------------------------------------------
//	SROM(on Device) Access Function
//--------------------------------------------------------------------------------

//
// Read SROM Request
//
int ReadSROMRequest(PXHCUTIL_SROMACC pAcc)
{
	DWORD dwSize;
	DWORD dwRet = ERROR_SUCCESS;
	BOOL bRet;
	int ret;

	// read request
	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_READ_SROM,
					pAcc,
					sizeof_XHCUTIL_SROMACC(0),
					NULL,
					0,
					&dwSize,
					NULL
					);

	if(!bRet){
		dwRet = GetLastError();
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_READ_SROM] fail (%d)\n"),dwRet);
	}

	ret = ConvertStatus(dwRet);

	return ret;
}


//
// Get Read Status
//
int GetReadSROMStatus(PXHCUTIL_GETRESULT pResult, DWORD* pLength)
{
	DWORD dwSize;
	DWORD dwRet = ERROR_SUCCESS;
	BOOL bRet;
	int ret;

	// read request
	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_GET_READSTATUS,
					pResult,
					sizeof_XHCUTIL_GETRESULT(0),
					pResult,
					sizeof_XHCUTIL_GETRESULT(0),
					&dwSize,
					NULL
					);

	if(!bRet){
		dwRet = GetLastError();
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_GET_READSTATUS] fail (%d)\n"),dwRet);
	
		ret = ConvertStatus(dwRet);
	}
	else{

		if(pLength){
			*pLength = pResult->Status2;
		}

		// command active
		if((pResult->Status & 0xFFFF) != 0){
			ret = XHCSTS_PROC_ACTIVE;
		}
		// Unknown Serial ROM
		else if((pResult->Status & 0xFFFF0000) == UFW_UNKNOWN_SROM){
			ret = XHCSTS_UNKNOWN_SROM;
		}
		// error 
		else if((pResult->Status & 0xFFFF0000) != 0){
			ret = XHCSTS_ERROR;
		}
		else{
			ret = XHCSTS_SUCCESS;
		}
	}

	return ret;

}


//
// Get Read Data
//
int GetReadSROMData(PXHCUTIL_SROMACC pAcc, UCHAR* pBuf, DWORD* pLength)
{
	DWORD dwSize;
	DWORD dwRet = ERROR_SUCCESS;
	BOOL bRet;
	int ret;

	// read request
	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_GET_READDATA,
					pAcc,
					sizeof_XHCUTIL_GETRESULT(0),
					pBuf,
					pAcc->Size,
					&dwSize,
					NULL
					);

	if(!bRet){
		dwRet = GetLastError();
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_GET_READDATA] fail (%d)\n"),dwRet);
	
		ret = ConvertStatus(dwRet);
	
	}
	else {
		ret = XHCSTS_SUCCESS;

		if(dwSize != pAcc->Size){
			DebugTrace(LITERAL("Warning read size mismatch (req:%d , result:%d)\n"),pAcc->Size,dwSize);
		}

		if(pLength){
			*pLength = dwSize;
		}

	}

	return ret;

}



//
// Read SROM
//
int ReadSROM(USHORT devAddr, 
			UINT offset, 
			ULONG size, 
			UCHAR *pBuf,
			ULONG *pReadSize,
			PXHC_SROM_INFO pSROMInfo
			)
{
	PXHCUTIL_SROMACC pAcc;
	PXHCUTIL_GETRESULT pResult;
	DWORD dwSize = 0;
	DWORD romSize;
	int ret;

	if(pReadSize){
		*pReadSize = 0;
	}

	romSize = GetSROMSize(pSROMInfo, TRUE);
	if(pSROMInfo){
/* There are A side and a B side, so the double size */
		if(MAX_SROM_DUMP_SIZE < size){
			return XHCSTS_PARAMETER_ERROR;
		}

		pAcc = MakeSROMRequest(	devAddr,
								offset,
								size,
								pSROMInfo,
								FALSE);
	}
	else{
		if(MAX_SROM_DUMP_SIZE < size){
			return XHCSTS_PARAMETER_ERROR;
		}

		pAcc = MakeSROMRequest(	devAddr,
								offset,
								size,
								&g_SROMInfo,
								FALSE);
	}


	if(!pAcc){
		return XHCSTS_MEMORY_ERROR;
	}

	
	// Read SROM Request
	ret = ReadSROMRequest(pAcc);

	if(ret != XHCSTS_SUCCESS){
		FreeSROMRequest(pAcc);
		return ret;
	}

	if(pBuf){

		pResult = (PXHCUTIL_GETRESULT)pAcc;

		do{
		
			// Get read status
			ret = GetReadSROMStatus(pResult,&dwSize);

			if(ret == XHCSTS_PROC_ACTIVE){
				SLEEP(SROM_POLL_WAITSLEEP);
			}
		}while(ret == XHCSTS_PROC_ACTIVE);

		if(ret== XHCSTS_SUCCESS){
			// Get read data
			pAcc->Size = dwSize;
			pAcc->Offset = 0;

			ret = GetReadSROMData(pAcc,pBuf,&dwSize);
		}
	}

	if(pReadSize){
		*pReadSize = dwSize;
	}


	FreeSROMRequest(pAcc);

	return ret;

}


//
// Get Read Data
//
int GetReadData(USHORT devAddr,
				ULONG size,
				UCHAR *pBuf,
				ULONG *pReadSize
				)
{

	PXHCUTIL_SROMACC pAcc;
	PXHCUTIL_GETRESULT pResult;
	DWORD dwSize = 0;
	int ret;


	if(pReadSize){
		*pReadSize = 0;
	}

	pAcc = MakeSROMRequest(	devAddr,
							0,
							size,
							NULL,
							FALSE);


	if(!pAcc){
		return XHCSTS_MEMORY_ERROR;
	}

	
	pResult = (PXHCUTIL_GETRESULT)pAcc;

	// Get read status
	ret = GetReadSROMStatus(pResult, &dwSize);

	if(ret== XHCSTS_SUCCESS){
		// Get read data
		pAcc->Size = size;
		pAcc->Offset = 0;

		ret = GetReadSROMData(pAcc, pBuf, &dwSize);
	
	}

	if(pReadSize){
		*pReadSize = dwSize;
	}

	FreeSROMRequest(pAcc);

	return ret;

}

// Write SROM Request
int WriteSROMRequest(PXHCUTIL_SROMACC pAcc)
{
	DWORD dwSize;
	DWORD dwRet = ERROR_SUCCESS;
	BOOL bRet;
	int ret;

	// write request
	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_WRITE_SROM,
					pAcc,
					sizeof_XHCUTIL_SROMACC(pAcc->Size),
					NULL,
					0,
					&dwSize,
					NULL
					);

	if(!bRet){
		dwRet = GetLastError();
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_WRITE_SROM] fail (%d)\n"),dwRet);
	}

	ret = ConvertStatus(dwRet);

	return ret;
}


// Erase SROM Request
int EraseSROMRequest(PXHCUTIL_SROMACC pAcc)
{
	DWORD dwSize;
	DWORD dwRet = ERROR_SUCCESS;
	BOOL bRet;
	int ret;

	// erase request
	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_ERASE_SROM,
					pAcc,
					sizeof_XHCUTIL_SROMACC(0),
					NULL,
					0,
					&dwSize,
					NULL
					);

	if(!bRet){
		dwRet = GetLastError();
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_ERASE_SROM] fail (%d)\n"),dwRet);
	}

	ret = ConvertStatus(dwRet);

	return ret;
}




//
// Get Write/Erase Status
//
int GetWriteSROMStatus(PXHCUTIL_GETRESULT pResult, DWORD* pLength)
{
	DWORD dwSize;
	DWORD dwRet = ERROR_SUCCESS;
	BOOL bRet;
	int ret;

	// read request
	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_GET_WRITESTATUS,
					pResult,
					sizeof_XHCUTIL_GETRESULT(0),
					pResult,
					sizeof_XHCUTIL_GETRESULT(0),
					&dwSize,
					NULL
					);

	if(!bRet){
		dwRet = GetLastError();
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_GET_WRITESTATUS] fail (%d)\n"),dwRet);
	
		ret = ConvertStatus(dwRet);
	}
	else{

		if(pLength){
			*pLength = pResult->Status2;
		}

		// command active
		if((pResult->Status & 0xFFFF) != 0){
			ret = XHCSTS_PROC_ACTIVE;
		}
		// error 
		else if((pResult->Status & 0xFFFF0000) != 0){
			ret = XHCSTS_ERROR;
		}
		else{
			ret = XHCSTS_SUCCESS;
		}
	}



	return ret;

}



//
// Write SROM
//
int WriteSROM(USHORT devAddr,
			  UINT offset,
			  ULONG size,
			  UCHAR* pBuf,
			  PXHC_SROM_INFO pSROMInfo,
			  BOOL wait)
{
	
	PXHCUTIL_SROMACC pAcc;
	PXHCUTIL_GETRESULT pResult;
	DWORD dwSize;
	DWORD romSize;
	int ret;

	romSize = GetSROMSize(pSROMInfo, TRUE);

	if(pSROMInfo){

		if(romSize < (offset + size)){
			return XHCSTS_PARAMETER_ERROR;
		}

		pAcc = MakeSROMRequest(	devAddr,
								offset,
								size,
								pSROMInfo,
								TRUE);
	}
	else{

		if(romSize < (offset + size)){
			return XHCSTS_PARAMETER_ERROR;
		}

		pAcc = MakeSROMRequest(	devAddr,
								offset,
								size,
								&g_SROMInfo,
								TRUE);
	}


	if(!pAcc){
		return XHCSTS_MEMORY_ERROR;
	}

	// Copy Write Data
	COPYMEMORY(pAcc->Data,pBuf,size);

	// Write SROM Request
	ret = WriteSROMRequest(pAcc);

	if(ret != XHCSTS_SUCCESS){
		FreeSROMRequest(pAcc);
		return ret;
	}

	if(wait){

		pResult = (PXHCUTIL_GETRESULT)pAcc;

		do{
		
			// Get write status
			ret = GetWriteSROMStatus(pResult,&dwSize);

			if(ret == XHCSTS_PROC_ACTIVE){
				SLEEP(SROM_POLL_WAITSLEEP);
			}

		}while(ret == XHCSTS_PROC_ACTIVE);
	}

	FreeSROMRequest(pAcc);

	return ret;

}

//
// Erase SROM
//
int EraseSROM(USHORT devAddr,
			  PXHC_SROM_INFO pSROMInfo,
			  BOOL wait)
{
	
	PXHCUTIL_SROMACC pAcc;
	PXHCUTIL_GETRESULT pResult;
	DWORD dwSize;
	int ret;

	if(pSROMInfo){
		pAcc = MakeSROMRequest(	devAddr,
								0,
								pSROMInfo->Size,
								pSROMInfo,
								FALSE);
	}
	else{
		pAcc = MakeSROMRequest(	devAddr,
								0,
								g_SROMInfo.Size,
								&g_SROMInfo,
								FALSE);
	}


	if(!pAcc){
		return XHCSTS_MEMORY_ERROR;
	}
	
	// Erase SROM Request
	ret = EraseSROMRequest(pAcc);

	if(ret != XHCSTS_SUCCESS){
		FreeSROMRequest(pAcc);
		return ret;
	}

	if(wait){

		pResult = (PXHCUTIL_GETRESULT)pAcc;

		do{
			// Get erase status
			ret = GetWriteSROMStatus(pResult,&dwSize);

			if(ret == XHCSTS_PROC_ACTIVE){
				SLEEP(SROM_POLL_WAITSLEEP);
			}

		}while(ret == XHCSTS_PROC_ACTIVE);
	}

	FreeSROMRequest(pAcc);

	return ret;

}

//
// Get Write/Erase Status
//
int GetWriteStatus(USHORT devAddr, ULONG *pReadSize)
{

	PXHCUTIL_SROMACC pAcc;
	PXHCUTIL_GETRESULT pResult;
	DWORD dwSize = 0;
	int ret;

	pAcc = MakeSROMRequest(	devAddr,
							0,
							0,
							NULL,
							FALSE);


	if(!pAcc){
		return XHCSTS_MEMORY_ERROR;
	}


	pResult = (PXHCUTIL_GETRESULT)pAcc;

	// Get erase status
	ret = GetWriteSROMStatus(pResult, &dwSize);

	if(pReadSize){
		*pReadSize = dwSize;
	}

	FreeSROMRequest(pAcc);

	return ret;

}


//--------------------------------------------------------------------------------
//	 Get SROM(on Device) Information Function
//--------------------------------------------------------------------------------

// Get Device Info From SROM
int GetDeviceInfo(USHORT devAddr, PXHC_DEVICE_INFO pDevInfo)
{
	ULONG getData;
	ULONG bootFwAddr;
	ULONG readTotalSize;
	ULONG LengthChk;
	ULONG readSize;
	ULONG getData_tmp;
	ULONG readFwImageSize;

	UCHAR buf[XHC_VSC_TMPBUFF_SIZE];

	USHORT getCrc;
	USHORT crcResult = 0;
	USHORT subSysVenId;
	USHORT subSysId;
	USHORT chkData;
	USHORT chkDataAdder;
	USHORT chkShiftSize;
	int buffPoint = 0;
	int loop;
	int dataLength;
	int romLoop;
	int ret = XHCSTS_SUCCESS;

	// Initialize
	LengthChk = XHC_SROM_ACCESS_SIZE;
	readTotalSize = 0;

	for(romLoop = 0; romLoop < SROM_SIDE_ALL; romLoop++){
		// Flag Init
		pDevInfo->VcdInfo[romLoop]   = FALSE;
		pDevInfo->SSIDInfo[romLoop]  = FALSE;
		pDevInfo->SSVIDInfo[romLoop] = FALSE;
		pDevInfo->EUIInfo[romLoop]   = FALSE;

		// Get Data Length
		ret = ReadSROM( devAddr,
						0,
						XHC_SROM_ACCESS_SIZE,
						(UCHAR*)&getData,
						&readSize,
						&g_SROMInfo
						);
		readFwImageSize = readSize;
		readTotalSize   = readSize;

		if(ret != XHCSTS_SUCCESS){
			DebugTrace(LITERAL("Read SROM Header fail.\n"));
			return ret;
		}

		for(loop = 0; loop < XHC_SROM_ACCESS_SIZE; loop++){
			buf[buffPoint++] = (UCHAR)(getData >> (loop*XHC_SHIFT_BIT_1BYTE));
		}

		/* SubSystem Vendor ID Check */
		if(((getData >> XHC_SROM_HIGH_BIT_SHIFT) & VSC_DATA_BLOCK_ADDRESS_MASK) == VSC_DATA_ADDR_SUBSYSTEM_VID){
			subSysVenId = (USHORT)(((getData >> XHC_SROM_HIGH_BIT_SHIFT) & VSC_DATA_BLOCK_DATA_MASK) >> VSC_DATA_SHIFT_BIT);
			pDevInfo->SSIDInfo[romLoop] = TRUE;
		}
		getData &= XHC_SROM_LOW_BIT_MASK;

		if(getData != SROM_START_CODE){	/* Data Length Check */
			if(getData < XHC_SROM_SIZE16K){
				pDevInfo->VcdInfo[romLoop] = TRUE;
				dataLength = (int)getData;

				/* Vendor Specific Configuration Block
				   Vendor Specific Configuration : 2*NByte(readTotalSize)
				   DataLength : 2byte
				   CRC        : 2byte
				*/

				while((dataLength+XHC_SROM_ACCESS_SIZE) >= (int)readTotalSize){
					ret = ReadSROM( devAddr,
									0,
									XHC_SROM_ACCESS_SIZE,
									(UCHAR*)&getData,
									&readSize,
									&g_SROMInfo
									);

					if(ret != XHCSTS_SUCCESS){
						DebugTrace(LITERAL("Read SROM Header fail.\n"));
						return ret;
					}


					if(dataLength > (int)readTotalSize){
						getData_tmp = getData;
						for(loop = 0; loop < XHC_SROM_ACCESS_SIZE; loop++){
							buf[buffPoint++] = (UCHAR)(getData_tmp >> (loop*XHC_SHIFT_BIT_1BYTE));
						}
					}
					else if(dataLength == (int)readTotalSize){
						getData_tmp = getData;
						for(loop = 0; loop < XHC_SROM_ACCESS_SIZE_SHORT; loop++){
							buf[buffPoint++] = (UCHAR)((getData_tmp & XHC_SROM_LOW_BIT_MASK) >> (loop*XHC_SHIFT_BIT_1BYTE));
						}
					}
					else{
						;
					}

					readTotalSize += XHC_SROM_ACCESS_SIZE;
					chkShiftSize = 0;

					while(chkShiftSize <= XHC_SROM_HIGH_BIT_SHIFT){
						chkData = (USHORT)((getData >> chkShiftSize) & XHC_SROM_LOW_BIT_MASK);
						chkDataAdder = (chkData & VSC_DATA_BLOCK_ADDRESS_MASK);

						if(chkDataAdder == (VSC_DATA_ADDR_SUBSYSTEM_VID+1)){	/* SubSystem Vendor ID(Hi-bit) Check */
							pDevInfo->SubSystemVendorID[romLoop] = (subSysVenId | (chkData & VSC_DATA_BLOCK_DATA_MASK));
						}
						else if(chkDataAdder == VSC_DATA_ADDR_SUBSYSTEM_ID){		/* SubSystem Vendor ID(Lo-bit) Check */
							subSysId  = ((chkData & VSC_DATA_BLOCK_DATA_MASK) >> VSC_DATA_SHIFT_BIT);
							pDevInfo->SSVIDInfo[romLoop] = TRUE;
						}
						else if(chkDataAdder == (VSC_DATA_ADDR_SUBSYSTEM_ID+1)){		/* SubSystem Vendor ID(Hi-bit) Check */
							pDevInfo->SubSystemID[romLoop] = (subSysId | (chkData & VSC_DATA_BLOCK_DATA_MASK));
						}
						else if(chkDataAdder == VSC_DATA_ADDR_SERIAL_NUM1){		/* Serial Number 1st Check(EUI-64) */
							pDevInfo->Eui64[romLoop][0] = ((chkData & VSC_DATA_BLOCK_DATA_MASK) >> VSC_DATA_SHIFT_BIT);
							pDevInfo->EUIInfo[romLoop] = TRUE;
						}
						else if(chkDataAdder == (VSC_DATA_ADDR_SERIAL_NUM1+1)){		/* Serial Number 1st Check(EUI-64) */
							pDevInfo->Eui64[romLoop][1] = ((chkData & VSC_DATA_BLOCK_DATA_MASK) >> VSC_DATA_SHIFT_BIT);
						}
						else if(chkDataAdder == (VSC_DATA_ADDR_SERIAL_NUM1+2)){		/* Serial Number 1st Check(EUI-64) */
							pDevInfo->Eui64[romLoop][2] = ((chkData & VSC_DATA_BLOCK_DATA_MASK) >> VSC_DATA_SHIFT_BIT);
						}
						else if(chkDataAdder == (VSC_DATA_ADDR_SERIAL_NUM1+3)){		/* Serial Number 1st Check(EUI-64) */
							pDevInfo->Eui64[romLoop][3] = ((chkData & VSC_DATA_BLOCK_DATA_MASK) >> VSC_DATA_SHIFT_BIT);
						}
						else if(chkDataAdder == (VSC_DATA_ADDR_SERIAL_NUM2)){		/* Serial Number 2nd Check(EUI-64) */
							pDevInfo->Eui64[romLoop][4] = ((chkData & VSC_DATA_BLOCK_DATA_MASK) >> VSC_DATA_SHIFT_BIT);
						}
						else if(chkDataAdder == (VSC_DATA_ADDR_SERIAL_NUM2+1)){		/* Serial Number 2nd Check(EUI-64) */
							pDevInfo->Eui64[romLoop][5] = ((chkData & VSC_DATA_BLOCK_DATA_MASK) >> VSC_DATA_SHIFT_BIT);
						}
						else if(chkDataAdder == (VSC_DATA_ADDR_SERIAL_NUM2+2)){		/* Serial Number 2nd Check(EUI-64) */
							pDevInfo->Eui64[romLoop][6] = ((chkData & VSC_DATA_BLOCK_DATA_MASK) >> VSC_DATA_SHIFT_BIT);
						}
						else if(chkDataAdder == (VSC_DATA_ADDR_SERIAL_NUM2+3)){		/* Serial Number 2nd Check(EUI-64) */
							pDevInfo->Eui64[romLoop][7] = ((chkData & VSC_DATA_BLOCK_DATA_MASK) >> VSC_DATA_SHIFT_BIT);
						}
						else{;}
						chkShiftSize += XHC_SROM_HIGH_BIT_SHIFT;
					}
				}

				crcResult = update_crc16(CRC_16, &buf[0], dataLength+XHC_SROM_ACCESS_SIZE_SHORT);

				/* when size of Vendor Specific Configuration field is Multiple byte of 4 */
				if((dataLength % XHC_SROM_ACCESS_SIZE) == 0){
					getCrc = (USHORT)((getData_tmp & XHC_SROM_HIGH_BIT_MASK) >> XHC_SROM_HIGH_BIT_SHIFT);
					if(getCrc == crcResult){
						getData &= XHC_SROM_LOW_BIT_MASK;
						LengthChk       = readFwImageSize;
					}
					else{
						DebugTrace(LITERAL("Vendor Specific CRC Error\n"));
						DebugTrace(LITERAL("Get = 0x%04X\n"), getCrc);
						DebugTrace(LITERAL("Expectation = 0x%04X\n"), crcResult);
						ret = XHCSTS_SROM_HEAD_ERROR;
						return ret;
					}
				}
				else{
					getCrc = (USHORT)(getData & XHC_SROM_LOW_BIT_MASK);
					if(getCrc == crcResult){
						getData = ((getData & XHC_SROM_HIGH_BIT_MASK) >> XHC_SROM_HIGH_BIT_SHIFT);
						readFwImageSize = (dataLength % XHC_SROM_ACCESS_SIZE);
						LengthChk       = readFwImageSize;
					}
					else{
						DebugTrace(LITERAL("Vendor Specific CRC Error\n"));
						DebugTrace(LITERAL("Get = 0x%04X\n"), getCrc);
						DebugTrace(LITERAL("Expectation = 0x%04X\n"), crcResult);
						ret = XHCSTS_SROM_HEAD_ERROR;
						return ret;
					}
				}
			}
			else{
				if(romLoop == 0){
					DebugTrace(LITERAL("Read SROM Data length fail.\n"));
					return ret;
				}
				else{
					// ROM B side is not data
					break;
				}

			}
		}

		if(getData == SROM_START_CODE){	// Setup Code Check
			ret = ReadSROM( devAddr,
							0,
							XHC_SROM_ACCESS_SIZE,
							(UCHAR*)&getData,
							&readSize,
							&g_SROMInfo
							);

			readFwImageSize += readSize;
			readTotalSize   += readSize;

			if(LengthChk == XHC_SROM_ACCESS_SIZE){
				bootFwAddr = (getData & XHC_SROM_LOW_BIT_MASK);
			}
			else{
				bootFwAddr = ((getData & XHC_SROM_HIGH_BIT_MASK) >> XHC_SROM_HIGH_BIT_SHIFT);
			}

			/* Until 2 bytes of head of BootFW Data is acquired. */
			while(bootFwAddr >= readFwImageSize){
				ret = ReadSROM( devAddr,
								0,
								XHC_SROM_ACCESS_SIZE,
								(UCHAR*)&getData,
								&readSize,
								&g_SROMInfo
								);

				readFwImageSize += readSize;
				readTotalSize   += readSize;
				if(ret != XHCSTS_SUCCESS){
					DebugTrace(LITERAL("Read FW Image version fail.\n"));
					break;
				}
			}
			if(LengthChk == XHC_SROM_ACCESS_SIZE){
				pDevInfo->FwVersion[romLoop][XHC_FW_VERSION_LOW]  = (UCHAR)(getData & XHC_SROM_LOW_BIT_MASK);
				pDevInfo->FwVersion[romLoop][XHC_FW_VERSION_HIGH] = (UCHAR)((getData & XHC_SROM_LOW_BIT_MASK) >> XHC_SHIFT_BIT_1BYTE);
			}
			else{
				pDevInfo->FwVersion[romLoop][XHC_FW_VERSION_LOW]  = (UCHAR)((getData & XHC_SROM_LOW_BIT_MASK) >> XHC_SROM_HIGH_BIT_SHIFT);
				pDevInfo->FwVersion[romLoop][XHC_FW_VERSION_HIGH] = (UCHAR)(((getData & XHC_SROM_LOW_BIT_MASK) >> XHC_SROM_HIGH_BIT_SHIFT) >> XHC_SHIFT_BIT_1BYTE);
			}
		}
		else{
			ret = XHCSTS_SROM_HEAD_ERROR;	// Header Error
		}

		while(readTotalSize < pDevInfo->Ebsz){
			ret = ReadSROM( devAddr,
							0,
							XHC_SROM_ACCESS_SIZE,
							(UCHAR*)&getData,
							&readSize,
							&g_SROMInfo
							);
			readTotalSize += XHC_SROM_ACCESS_SIZE;
		}
		buffPoint = 0;
	}

	if(ret == XHCSTS_SUCCESS){
		ret = GetSROMIdentify(devAddr, &pDevInfo->SROMInfo, TRUE);
	}

	return ret;

}



//
// Detect Serial ROM
//
int GetSROMIdentify(USHORT devAddr, PXHC_SROM_INFO pSROMInfo, BOOL wait)
{
	PXHCUTIL_SROMACC pAcc;
	DWORD dwRet = ERROR_SUCCESS;
	DWORD dwSize = 0;
	BOOL bRet;
	int ret;

	pAcc = MakeSROMRequest(	devAddr,
							0,
							0,
							NULL,
							FALSE);
	if(!pAcc){
		return XHCSTS_MEMORY_ERROR;
	}
	pAcc->DevCode = 0;

	// request
	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_GET_SROM_IDENTIFY,
					pAcc,
					sizeof_XHCUTIL_SROMACC(0),
					pAcc,
					sizeof_XHCUTIL_SROMACC(0),
					&dwSize,
					NULL
					);

	if(!bRet){
		dwRet = GetLastError();
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_GET_SROM_IDENTIFY] fail (%d)\n"),dwRet);
	}

	ret = ConvertStatus(dwRet);

	if(ret != XHCSTS_SUCCESS){
		FreeSROMRequest(pAcc);
		return ret;
	}

	if(ret == XHCSTS_SUCCESS){
		pSROMInfo->Size = 0;
		pSROMInfo->Page = 0;
		pSROMInfo->Code = pAcc->DevCode;
	}

	FreeSROMRequest(pAcc);

	return ret;
}

//
// Set Serial ROM parameter
//
int SetSROMIdentify(USHORT devAddr, PXHC_SROM_INFO pSROMInfo)
{
	PXHCUTIL_SROMACC pAcc;
	DWORD dwSize;
	BOOL bRet;
	DWORD dwRet = ERROR_SUCCESS;
	int ret;

	if(pSROMInfo){
		pAcc = MakeSROMRequest(	devAddr,
								0,
								pSROMInfo->Size,
								pSROMInfo,
								FALSE);
	}
	else{
		pAcc = MakeSROMRequest(	devAddr,
								0,
								g_SROMInfo.Size,
								&g_SROMInfo,
								FALSE);
	}

	if(!pAcc){
		return XHCSTS_MEMORY_ERROR;
	}

	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_SET_SROM_IDENTIFY,
					pAcc,
					sizeof_XHCUTIL_SROMACC(0),
					NULL,
					0,
					&dwSize,
					NULL
					);

	if(!bRet){
		dwRet = GetLastError();
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_SET_SROM_IDENTIFY] fail (%d)\n"),dwRet);
	}

	ret = ConvertStatus(dwRet);
	FreeSROMRequest(pAcc);

	return ret;
}



//
// Set ExtROM Access Enable
//
int SetExtROMAccessEnable(USHORT devAddr, USHORT en)
{
	DWORD dwSize;
	DWORD dwRet = ERROR_SUCCESS;
	PXHCUTIL_SROMACC pAcc;
	BOOL bRet;
	int ret;

	pAcc = MakeSROMRequest(	devAddr,
							0,
							0,
							NULL,
							FALSE);

	if(!pAcc){
		return XHCSTS_MEMORY_ERROR;
	}

	if(en == XHC_EXTROM_ACCESS_ENABLE){
		// read request
		bRet = CallDeviceIOControl(
						g_hDriver,
						IOCTL_SET_EXTROM_ACCESS_ENABLE,
						pAcc,
						sizeof_XHCUTIL_GETRESULT(0),
						NULL,
						0,
						&dwSize,
						NULL
						);
		if(!bRet){
			dwRet = GetLastError();
			DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_SET_EXTROM_ACCESS_ENABLE] fail (%d)\n"),dwRet);
		}
	}
	else{
		bRet = CallDeviceIOControl(
						g_hDriver,
						IOCTL_SET_EXTROM_ACCESS_DISABLE,
						pAcc,
						sizeof_XHCUTIL_GETRESULT(0),
						NULL,
						0,
						&dwSize,
						NULL
						);
		if(!bRet){
			dwRet = GetLastError();
			DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_SET_EXTROM_ACCESS_DISABLE] fail (%d)\n"),dwRet);
		}
	}

	ret = ConvertStatus(dwRet);
	FreeSROMRequest(pAcc);

	return ret;
}

//
// Get SROM Write Result code
//
int GetROMWriteResultCode(USHORT devAddr, ULONG* pResultCode)
{
	DWORD dwSize;
	DWORD dwRet = ERROR_SUCCESS;
	PXHCUTIL_SROMACC pAcc;
	PXHCUTIL_GETRESULT pResult;
	BOOL bRet;
	int ret;

	pAcc = MakeSROMRequest(	devAddr,
							0,
							0,
							NULL,
							FALSE);

	if(!pAcc){
		return XHCSTS_MEMORY_ERROR;
	}

	pResult = (PXHCUTIL_GETRESULT)pAcc;

	// read request
	bRet = CallDeviceIOControl(
					g_hDriver,
					IOCTL_GET_SROM_RESULT_CODE,
					pAcc,
					sizeof_XHCUTIL_GETRESULT(0),
					pAcc,
					sizeof_XHCUTIL_GETRESULT(0),
					&dwSize,
					NULL
					);

	if(!bRet){
		dwRet = GetLastError();
		DebugTrace(LITERAL("CallDeviceIOControl[IOCTL_GET_SROM_RESULT_CODE] fail (%d)\n"),dwRet);
	}
	else{
		*pResultCode = pResult->Status2;
	}

	ret = ConvertStatus(dwRet);
	FreeSROMRequest(pAcc);

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//		CRC Function
///////////////////////////////////////////////////////////////////////////////
USHORT update_crc16(USHORT crc, const UCHAR *t, int len)
{
    int n;
    USHORT c = crc ^ 0xffffU;
    BYTE m;

    for(n = 0; n < len; n++) {
        m = t[n];
        c = crc_table[(c ^ m) & 0xf] ^ (c >> 4);
        c = crc_table[(c ^ (m >> 4)) & 0xf] ^ (c >> 4);
    }

    return c ^ 0xffffU;
}
