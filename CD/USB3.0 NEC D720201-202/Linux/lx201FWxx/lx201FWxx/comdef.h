/*
	File Name : comdef.h
	
	Common define

	* Copyright (C) 2011 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2011-05-20	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _COMDEF_H_
#define _COMDEF_H_

//********************************************************************************//
//	compile switch 
//********************************************************************************//

//********************************************************************************//
//	define
//********************************************************************************//


//
//	Target Device Revision ID
#define TARGET_REVISION_1		0x01
#define W201_TARGET_REVISION_2	0x02
#define W201_TARGET_REVISION_3	0x03

#define W201_TARGET_REVISION	(W201_TARGET_REVISION_3)  // latest version
#define W202_TARGET_REVISION	0x02

//
// Return Code
//

#define W201STS_SUCCESS 			0
#define W201STS_ERROR 				-1
#define W201STS_MEMORY_ERR			-2
#define W201STS_EXE_MISMATCH		-3
#define W201STS_CMDLINE_ERR			-4
#define W201STS_DEVACCESS_ERR		-5
#define W201STS_DEVICE_FOUND_ERR	-6
#define W201STS_FILE_NOT_FOUND		-7
#define W201STS_VERIFY_ERR			-8
#define W201STS_FWFILE_ERR			-9
#define W201STS_CONFIGFILE_ERR		-10
#define W201STS_EUI64FILE_ERR		-11
#define W201STS_IMGFILE_ERR			-12
#define W201STS_LOGFILE_ERR			-13
#define W201STS_TMPFILE_ERR			-14
#define W201STS_SROMFORMAT_ERR		-15
#define W201STS_REVISION_ERR		-16
#define W201STS_UNKNOWN_SROM		-17

//
// Command Code
//
#define W201CMD_WRITE				0x00000001
#define W201CMD_ERASE				0x00000002
#define W201CMD_DUMP				0x00000040
#define W201CMD_IMAGEGEN			0x00000080
#define W201CMD_VERIFY				0x00000100
#define W201CMD_WRITEALL			0x00000200

#define W201CMD_ENUM				0x00001000
#define W201CMD_USAGE				0x00004000
#define W201CMD_EXUSAGE				0x00008000

#define W201CMD_ADDRESS				0x00010000
#define W201CMD_NOERASE				0x00040000
#define W201CMD_NOREV				0x01000000
#define W201CMD_ROMTYPE				0x02000000
#define W201CMD_ROMINFO				0x04000000
#define W201CMD_LOG					0x08000000


#define W201CMD_MASK				0x000013FF	// Use xhc201w.dll command mask
#define W201CMD_EXMASK				0x0000D3FF	// Extended valid command mask
#define W201CMD_EXMASK2				0x00FFFFFF	// Extended valid command mask 2

#define W201CMD_ERSMASK1			0x000013FD	// command mask except /Erase
#define W201CMD_ERSMASK2			0x0000023d	// /Erase enable command mask

#define W201CMD_NOERSMASK			0x0000023d	// /noerase enable command mask

#define W201CMD_SROMMASK			0x0000037F	// /srom or /srominfo mask

#define W201CMD_FOUTMASK			0x0000001D	// /fout mask

#define W201CMD_ADDRMASK			0x000001FF	// /address mask

#define W201CMD_NOREVMASK			0x000013FF	// /norev mask

//
// Device Reset
//
#define W201CMD_RESET_DEVICE		1		// 0: Don't reset when device close.  1: Run reset when device close.

//
// Access Enable param
//
#define XHC_EXTROM_ACCESS_ENABLE	1
#define XHC_EXTROM_ACCESS_DISABLE	0

//------------------------------------------------------------------------------------
// SROM 
//------------------------------------------------------------------------------------

#define SROM_PAGE_SIZE		128		// default Page size
#define SROM_CODE_FROM		0x00E0	// default device Code


// SROM Device Code
#define ROMCODE_ERASE		0xFF00	// Erase Command
#define ROMCODE_TYPE		0x0040	// 0:EEPROM, 1:FlashMemory
#define ROMCODE_ADDR		0x0020	// 0:2Byte addressing, 1:3Byte addressing


#define W201_VENDOR_ID		0x1912
#define W202_VENDOR_ID		0x1912

#define W201_DEVICE_ID		0x14
#define W202_DEVICE_ID		0x15

#define NONE_DEVICE_ID		0xFF

// Chip Erase
#define SROM_ERASE_BLOCK_SIZE_4K		 0x1000UL
#define SROM_ERASE_BLOCK_SIZE_32K		 0x8000UL
#define SROM_ERASE_BLOCK_SIZE_64K		0x10000UL
#define SROM_ERASE_BLOCK_SIZE_128K		0x20000UL

#define SROM_ERASE_BLOCK_MASK			   0x30
#define SROM_ERASE_BLOCK_SHIFT_BIT		   4


// ProductID
#define PRODUCT_NAME_AT25F512B			0x001F6500UL
#define PRODUCT_NAME_25Lxx				0x00C22000UL	/* 0x00C220XX */
#define PRODUCT_NAME_PM25LD				0x009D7F00UL	/* 0x009D7FXX */
#define PRODUCT_NAME_W25XxxV			0x00EF3000UL	/* 0x00EF30XX */
#define PRODUCT_NAME_A25Lxxx			0x00373000UL	/* 0x003730XX */
#define PRODUCT_NAME_25Lxx21E			0x00C22200UL	/* 0x00C222XX */
#define PRODUCT_NAME_M25P10A			0x00202010UL
#define PRODUCT_NAME_M25P11A			0x00202011UL
#define PRODUCT_NAME_M25LV512A			0x009D7F7BUL
#define PRODUCT_NAME_M25LV010A			0x009D7F7CUL
#define PRODUCT_NAME_M25P12				0x00202012UL
#define PRODUCT_NAME_M25P13				0x00202013UL
#define PRODUCT_NAME_M25LV020A			0x009D7F7DUL
#define PRODUCT_NAME_M25LV040A			0x009D7F7EUL
#define PRODUCT_NAME_SST25VF048A		0x00BF0048UL
#define PRODUCT_NAME_SST25VF049A		0x00BF0049UL
#define PRODUCT_NAME_SST25VF043A		0x00BF0043UL
#define PRODUCT_NAME_SST25VFxxxB		0x00BF2500UL	/* 0x00BF25XX */
#define PRODUCT_NAME_R1EX25512ASA00A	0x00FF0000UL

#define PRODUCT_NAME_xx_MASK			0xFFFFFF00UL
#define PRODUCT_ERROR_PARAM				0xFFFFFFFFUL

//------------------------------------------------------------------------------------
// etc 
//------------------------------------------------------------------------------------
#define DEF_LOG_NAME				LITERAL("log.txt")


#define STR_APPNAME_LEN		9		// application name (8+NULL)
#define STR_ADDRESS_LEN		9		// PCI Address XX:XX:XX (8+NULL)

#define READ_POLL_INTERVAL		100		// Polling Interval(ms)	
#define WRITE_POLL_INTERVAL		100		// Polling Interval(ms)	


#define FWREV_NOCHECK			0xFF	// Use DeviceOpen

#define	SROM_START_CODE 	0x55AA
#define SROM_SHIFT_BIT_1BYTE		8
#define SROM_ACCESS_SIZE		4	// Byte
#define SROM_ACCESS_SIZE_SHORT	2	// Byte

#define SROM_READSIZE				2
#define SROM_READSIZE_EBSZ_4BYTE	8

//********************************************************************************//
//	struct
//********************************************************************************//


// command
typedef struct _COMMAND_INFO{
	ULONG cmd;
	TCHAR fname1[MAX_PATH];
	TCHAR fname2[MAX_PATH];
	TCHAR fname3[MAX_PATH];
	TCHAR fname4[MAX_PATH];
	TCHAR fname5[MAX_PATH];
	USHORT devAddr;
	UINT srom;
}COMMAND_INFO,*PCOMMAND_INFO;

// Status Convert Entry
typedef struct _STATUS_CONV_ENTRY{
	int	libsts;	// Library Code
	int	appsts;	// Appplication Status
}STATUS_CONV_ENTRY,*PSTATUS_CONV_ENTRY;


//********************************************************************************//
//	enum
//********************************************************************************//
typedef enum {
	PCI_REG_RESULT_CODE_INVALID = 0,
	PCI_REG_RESULT_CODE_SUCCESS,
	PCI_REG_RESULT_CODE_ERROR,
	PCI_REG_RESULT_CODE_ALL
} PCI_REG_RESULT_CODE;

typedef enum {
	SROM_EBSZ_4KBYTE = 0,
	SROM_EBSZ_32KBYTE,
	SROM_EBSZ_64KBYTE,
	SROM_EBSZ_128KBYTE,
	SROM_EBSZ_ALL
} SROM_EBSZ;

//********************************************************************************//
//	macro
//********************************************************************************//


// Get high bit(7-4) from Byte
#define GetByteHi(x) 			((((unsigned char)x) & 0xF0) >> 4)
// Get low bit(3-0) from Byte
#define GetByteLo(x) 			(((unsigned char)x) & 0x0F)


#endif // #ifndef _COMDEF_H_
