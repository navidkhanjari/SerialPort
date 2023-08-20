/*
	File Name : sromio.h
	
	Serial ROM I/O 

	* Copyright (C) 2010 Renesas Electronics Corporation
	* Copyright (c) 2009 NEC Engineering, Ltd.

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2009-10-10	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _SROMIO_H_
#define _SROMIO_H_



//********************************************************************************//
//	define
//********************************************************************************//

// SROM access polling wait time
#define SROM_POLL_WAITSLEEP			  100	// ms

//********************************************************************************//
//	struct
//********************************************************************************//

#pragma pack(1)

// SROM Block Entry in SROM Header
typedef struct _SROM_BLOCK_ENTRY{
	USHORT Addr;
	USHORT Size;
}SROM_BLOCK_ENTRY,*PSROM_BLOCK_ENTRY;

#pragma pack()


//********************************************************************************//
//	Function
//********************************************************************************//


//--------------------------------------------------------------------------------
//	SROM Image (On Memory) Access
//--------------------------------------------------------------------------------

// Get SROM size 
ULONG GetSROMSize(PXHC_SROM_INFO pSROMInfo, BOOL bDevice);

// Update SROM Image
int UpdateSROMImage(
				UCHAR* pImageData,
				ULONG size,
				USHORT subsysvenID,
				USHORT subsysID,
				UCHAR* eui64,
				PXHC_SROM_INFO pSROMInfo,
				ULONG flags
				);

int UpdateSROMImageFromFile(
			LPCTSTR lpMemPath,
			LPCTSTR lpCfgPath,
			LPCTSTR lpEui64Path,
			PXHC_SROM_INFO pSROMInfo,
			UCHAR* pImageData
		   );

// Read SROM
int ReadSROM(USHORT devAddr, 
			UINT offset, 
			ULONG size, 
			UCHAR *pBuf,
			ULONG *pReadSize,
			PXHC_SROM_INFO pSROMInfo
			);

// Get Read Data
int GetReadData(USHORT devAddr,
				ULONG size,
				UCHAR *pBuf,
				ULONG *pReadSize
				);

// Write SROM
int WriteSROM(USHORT devAddr,
			  UINT offset,
			  ULONG size,
			  UCHAR* pBuf,
			  PXHC_SROM_INFO pSROMInfo,
			  BOOL wait);


// Erase SROM
int EraseSROM(USHORT devAddr,
			  PXHC_SROM_INFO pSROMInfo,
			  BOOL wait);

// Get Write/Erase Status
int GetWriteStatus(USHORT devAddr, ULONG *pReadSize);

// Get Device Information from Device
int GetDeviceInfo(USHORT devAddr, PXHC_DEVICE_INFO pDevInfo);

// Serial ROM Identify
int GetSROMIdentify(USHORT devAddr, PXHC_SROM_INFO pSROMInfo, BOOL wait);

// Set Serial ROM parameter
int SetSROMIdentify(USHORT devAddr, PXHC_SROM_INFO pSROMInfo);

// Set ExtROM Access Enable
int SetExtROMAccessEnable(USHORT devAddr, USHORT en);

// Get SROM Write Result code
int GetROMWriteResultCode(USHORT devAddr, ULONG* pResultCode);

// CRC16
USHORT update_crc16(USHORT crc, const BYTE *t, int len);


#endif // #ifndef _SROMIO_H_


