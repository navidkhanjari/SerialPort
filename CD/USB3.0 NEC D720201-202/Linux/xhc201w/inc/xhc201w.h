/*
	File Name : xhc201w.h
	
	Export API for uPD720200 

	* Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//      2010-08-16  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////


#ifndef _XHC201W_H_
#define _XHC201W_H_


#ifdef _cplusplus
extern "C" {
#endif // #ifdef _cplusplus

#define XHC201W_API
#define _API

	
//********************************************************************************//
//	define
//********************************************************************************//

//------------------------------------------------------------------------------------
// API Result Code  
//------------------------------------------------------------------------------------

#define XHCSTS_SUCCESS				0		// success
#define XHCSTS_ERROR				-1		// fail (unknown error)

// general error
#define XHCSTS_FIND_DEVICE_ERROR	-11
#define XHCSTS_NOT_OPENED			-12
#define XHCSTS_ALREADY_OPEN			-13
#define XHCSTS_BUSY					-14
#define XHCSTS_PARAMETER_ERROR		-15
#define XHCSTS_MEMORY_ERROR			-16
#define XHCSTS_INVALID_REQUEST		-17

// UFW error
#define XHCSTS_LOADUFW_ERROR		-21
#define XHCSTS_DEVRESET_ERROR		-22

// driver load/unload error
#define XHCSTS_NOT_INITIALIZE		-31
#define XHCSTS_DRVLOAD_ERROR		-32
#define XHCSTS_DRVUNLOAD_ERROR		-33
#define XHCSTS_DEVMGR_ERROR			-34
#define XHCSTS_CHDCHK_ERROR			-35

// file error
#define XHCSTS_MEMFILE_NOT_FOUND	-41
#define XHCSTS_MEMFILE_ERR			-42
#define XHCSTS_CFGFILE_NOT_FOUND	-43
#define XHCSTS_CFGFILE_ERR			-44
#define XHCSTS_EUIFILE_NOT_FOUND	-45
#define XHCSTS_EUIFILE_ERR			-46
#define XHCSTS_FWFILE_NOT_FOUND		-47
#define XHCSTS_FWFILE_ERR			-48

// SROM error
#define XHCSTS_SROM_HEAD_ERROR		-51
#define XHCSTS_SROM_FOOTER_ERROR	-52
#define XHCSTS_SROM_FORMAT_ERROR	-53
#define XHCSTS_SROM_SIZE_OVER		-54
#define XHCSTS_UNKNOWN_SROM			-55

// xHCI error 
#define XHCSTS_INVALID_STATE		-61
#define XHCSTS_XHCIRST_ERROR		-62


// Active status
#define XHCSTS_PROC_ACTIVE			1	// Process Active
#define XHCSTS_UNCONNECTED			2	// Can't detect USB device connected to Port 


//------------------------------------------------------------------------------------
//	Device Open Mode ( Use xHcOpenDevice )
//------------------------------------------------------------------------------------
#define XHC_OPMODE_NORMAL			0x00000001	// normal open
#define XHC_OPMODE_UFW				0x00000002	

#define XHC_OPMODE_NODISABLE		0x00010000	// with no device disable
#define XHC_OPMODE_CHDCHK			0x00100000	// with child device check

//------------------------------------------------------------------------------------
//	SROM Access wait ( Use xHcReadSerialROM, xHcWriteSerialROM, xHcEraseSerialROM)
//------------------------------------------------------------------------------------
#define XHC_WAIT_COMPLETE			0	// wait complete
#define XHC_NO_WAIT					1	// no wait

//------------------------------------------------------------------------------------
//	SROM Image Update flag ( Use xHcUpdateROMImage )
//------------------------------------------------------------------------------------
#define XHC_UPD_SSID			0x1	// SubsystemVendorID & SubsystemID
#define XHC_UPD_EUI64			0x2	// EUI-64
#define XHC_UPD_SROMINFO		0x4	// Serial ROM Information

#define XHC_UPD_ALL             (XHC_UPD_SSID | XHC_UPD_EUI64 | XHC_UPD_SROMINFO)

//------------------------------------------------------------------------------------
// SROM 
//------------------------------------------------------------------------------------
#define MAX_SROM_SIZE				0x8000		// MAX Serial ROM size = 32k byte

#define XHC_SROM_ACCESS_SIZE	4	// Byte
#define XHC_SROM_ACCESS_SIZE_SHORT	2

#define MAX_SROM_DUMP_SIZE			0x40000		// MAX Serial ROM dump size = 128k byte

//********************************************************************************//
//	enum
//********************************************************************************//
typedef enum {
	DEVICE_ID_TYPE_D201 = 0,
	DEVICE_ID_TYPE_D202,
	DEVICE_ID_TYPE_ALL
} DEVICE_ID_TYPE;

typedef enum {
	SROM_SIDE_A = 0,
	SROM_SIDE_B,
	SROM_SIDE_ALL
} SROM_SIDE_TYPE;

//********************************************************************************//
//	struct
//********************************************************************************//

#pragma pack(1)

// PCI(PCI-Express) Device Information
typedef struct _XHC_PCIDEV_INFO{
	unsigned short DevAddr;				// device address
	unsigned short SubSystemVendorID;	// Subsystem Vendor ID (from Configuration Register)
	unsigned short SubSystemID;			// Subsytem ID (from Configuration Register)
	unsigned char RevisionID;			// Revision ID (from Configuration Register)
}XHC_PCIDEV_INFO,*PXHC_PCIDEV_INFO;

// Serial ROM Information
typedef struct _XHC_SROM_INFO{
	unsigned long Size;			// SROM size
	unsigned short Page;		// SROM page size
	unsigned long Code;		// SROM access code
}XHC_SROM_INFO,*PXHC_SROM_INFO;

typedef struct _SROM_TYPE_TABLE{
	ULONG SromType;
	ULONG Parameter;
	ULONG ProductId;
}SROM_TYPE_TABLE,*PSROM_TYPE_TABLE;

// Device Information (from Serial ROM)
typedef struct _XHC_DEVICE_INFO{
	unsigned short SubSystemVendorID[SROM_SIDE_ALL];// Subsystem Vendor ID ("A side" and "B side")
	unsigned short SubSystemID[SROM_SIDE_ALL];		// Subsytem ID("A side" and "B side")
	unsigned char FwVersion[SROM_SIDE_ALL][4];		// FW Version
	unsigned char Eui64[SROM_SIDE_ALL][8];			// EUI-64
	ULONG Ebsz;										// Erase Block Size
	BOOL VcdInfo[SROM_SIDE_ALL];					// VDC check flag
	BOOL SSIDInfo[SROM_SIDE_ALL];					// SSID check flag
	BOOL SSVIDInfo[SROM_SIDE_ALL];					// SSVID check flag
	BOOL EUIInfo[SROM_SIDE_ALL];					// EUI check flag
	XHC_SROM_INFO SROMInfo;							// SROM Information
}XHC_DEVICE_INFO,*PXHC_DEVICE_INFO;

// PHY Setting
typedef struct _XHC_CFG_SETTING_INFO{
	ULONG phySetting1;	/* [0]:parameter, [1]:Address */
	ULONG phySetting2;
	ULONG phySetting3;
	ULONG chipSetting;
}XHC_CFG_SETTING_INFO,*PXHC_CFG_SETTING_INFO;

#pragma pack()

//********************************************************************************//
//	macro
//********************************************************************************//

// Get Device Address (from BusNumber & DeviceNumber & FunctionNumber)
#define MakeDevAddr(bus,dev,func)		((bus&0x00FF)<<8)|((dev&0x001F)<<3)|(func&0x0007)

// Get BusNumber (from Device Address)
#define GetBus(devAddr)					((devAddr >>8) & 0xFF)
// Get BusNumber (from Device Address)
#define GetDev(devAddr)					((devAddr >> 3) & 0x1F)
// Get BusNumber (from Device Address)
#define GetFunc(devAddr)				(devAddr & 0x07)


//********************************************************************************//
//	API
//********************************************************************************//

/*
	Common Function
*/


// Library Initialize
XHC201W_API int _API xHcInitialize(void);
// Library Terminate
XHC201W_API int _API xHcTerminate(void);

// Search Target Device
XHC201W_API int _API xHcFindDevice(
					unsigned short venID,
					unsigned short devID,
					PXHC_PCIDEV_INFO pDevInfo,
					int num,
					unsigned int *pFindCount
					);


// Open Target Device
XHC201W_API int _API xHcOpenDevice(
					unsigned short devAddr,
					unsigned int mode,
                    PXHC_SROM_INFO pSROMInfo,
                    ULONG targetRevID
					);

// Close Target Device
XHC201W_API int _API xHcCloseDevice(unsigned short devAddr);


/*
	Serial ROM Function
*/

// Auto Detect Serial ROM
XHC201W_API int _API xHcDetectSerialROM(
					unsigned short devAddr,
					PXHC_SROM_INFO pSROMInfo
					);

// Set Serial ROM Information
XHC201W_API int _API xHcSetSerialROMInfo(
					unsigned short devAddr,
					PXHC_SROM_INFO pSROMInfo
					);


// Get Device Information from Device
XHC201W_API int _API xHcGetDeviceInfo(
					unsigned short devAddr,
					PXHC_DEVICE_INFO pDevInfo
					);


// Read Serial ROM
XHC201W_API int _API xHcReadSerialROM(
					unsigned short devAddr,
					unsigned int offset,
					unsigned long size,
					unsigned char *pReadData,
					unsigned long *pReadReturned,
					int comp
					);


// Get Read Data
XHC201W_API int _API xHcGetReadData(
					unsigned short devAddr,
					unsigned long size,
					unsigned char *pReadData,
					unsigned long *pReadReturned
					);


// Write Serial ROM 
XHC201W_API int _API xHcWriteSerialROM(
					unsigned short devAddr,
					unsigned long size,
					unsigned char *pWriteData,
					int comp
					);


// Erase Serial ROM
XHC201W_API int _API xHcEraseSerialROM(
					unsigned short devAddr,
					int comp
					);


// Get Write/Erase Status
XHC201W_API int _API xHcGetWriteStatus(
					unsigned short devAddr,
					unsigned long *pWriteSize
					);

// Set ExtROM Access Enable
XHC201W_API int _API xHcSetExtROMAccessEnable(
					unsigned short devAddr,
					unsigned short en
					);

// Get SROM Write Result code
XHC201W_API int _API xHcGetROMWriteResultCode(
					unsigned short devAddr,
					unsigned long* pResultCode
					);


/*
	Serial ROM Image Function
*/

// Update Serial ROM Image from file 
XHC201W_API int _API xHcUpdateROMImageFromFile(
					LPCWSTR lpMemPath,
					LPCWSTR lpCfgPath,
					LPCWSTR lpEui64Path,
					PXHC_SROM_INFO pSROMInfo,
					unsigned char* pImageData
					);


XHC201W_API int _API xHcResetDevice(
					unsigned short devAddr
					);

#ifdef _cplusplus
}
#endif // #ifdef _cplusplus

#endif // #ifndef _XHC201W_H_

