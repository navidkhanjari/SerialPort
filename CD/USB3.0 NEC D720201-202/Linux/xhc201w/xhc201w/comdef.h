/*
	File Name : comdef.h
	
	Common define

	* Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//      2010-08-16  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _COMDEF_H_
#define _COMDEF_H_


//********************************************************************************//
//	define
//********************************************************************************//

// Module Name
#define MODULE_NAME         LITERAL("libxhc201")

// ServiceName
#define DRVSVC_NAME         LITERAL("xhc201")

//------------------------------------------------------------------------------------
//	Driver File Name
//------------------------------------------------------------------------------------
#define XHCDRV_FILE_NAME    LITERAL("./xhc201")

#define XHCDRV_FILE_OPEN_ERR  -1


//------------------------------------------------------------------------------------
// HW   
//------------------------------------------------------------------------------------

//	Target Device Revision ID

// default device
#define XHC201_VENDOR_ID	0x1912	// vendor ID
#define XHC201_DEVICE_ID	0x0014	// vendor ID

#define XHC202_VENDOR_ID	0x1912	// vendor ID
#define XHC202_DEVICE_ID	0x0015	// vendor ID

// Vendor Specific Configuration address
#define VSC_DATA_ADDR_SUBSYSTEM_VID		0x0
#define VSC_DATA_ADDR_SUBSYSTEM_ID		0x2
#define VSC_DATA_ADDR_SERIAL_NUM1		0x4
#define VSC_DATA_ADDR_SERIAL_NUM2		0x8
#define VSC_DATA_ADDR_PHY_SETTING1		0xC
#define VSC_DATA_ADDR_PHY_SETTING2		0x10
#define VSC_DATA_ADDR_PHY_SETTING3		0x14
#define VSC_DATA_ADDR_CHIP_SETTING		0x18

#define VSC_DATA_BLOCK_ADDRESS_MASK		0x00ff
#define VSC_DATA_BLOCK_DATA_MASK		0xff00
#define VSC_DATA_SHIFT_BIT				8

//------------------------------------------------------------------------------------
// SROM 
//------------------------------------------------------------------------------------
#define	SROM_START_CODE 	0x55AA
#define	SROM_END_CODE 		0x55AA

#define SROM_SIG_SIZE		2			// signature size


// default param
#define XHC201_SROM_SIZE	0x8000		// size = 32k byte
#define XHC201_SROM_PAGE	0x0080		// page = 128 byte
#define XHC201_SROM_CODE	0x00E0		// code = flash

// Default Config Version
#define FWID_LD_SIZE		2			// FW Load Size area size

#define XHC_SROM_HIGH_BIT_MASK	0xFFFF0000UL
#define XHC_SROM_LOW_BIT_MASK	0x0000FFFFUL
#define XHC_SROM_HIGH_BIT_SHIFT	16

#define XHC_EXTROM_ACCESS_ENABLE	1
#define XHC_EXTROM_ACCESS_DISABLE	0

#define XHC_FW_VERSION_LOW		2
#define XHC_FW_VERSION_HIGH		3

#define XHC_SHIFT_BIT_1BYTE		8

#define XHC_SROM_SIZE32K		0x8000
#define XHC_SROM_SIZE16K		0x4000

#define XHC_VSC_TMPBUFF_SIZE	0x100
//------------------------------------------------------------------------------------
// BCD & ACD
//------------------------------------------------------------------------------------

// Configuration Block Size
#define DEF_CONFIG_BLOCKSIZE		(4*1024) // 4KB


#define BCD_START_OFFSET		(sizeof(SROMIMAGE_HEADER_EX))

#define BCD_ID_OFFSET			0
#define BCD_SIZE_OFFSET			1
#define BCD_HEADER_SIZE			2
#define BCD_BLOCK_MAX			256


// BCD : Subsystem Vender ID & Subsystem ID
#define CFGBASE_SUBSYS_ID		0x60
#define CFGADDR_SUBVEN_ID0		0x2C
#define CFGADDR_SUBVEN_ID1		0x2D
#define CFGADDR_SUBSYS_ID0		0x2E
#define CFGADDR_SUBSYS_ID1		0x2F
// BCD : Subsystem Vender ID & Subsystem ID Mirror
#define CFGBASE_MSUBSYS_ID		0x69
#define CFGADDR_MSUBVEN_ID0		0x68
#define CFGADDR_MSUBVEN_ID1		0x69
#define CFGADDR_MSUBSYS_ID0		0x6A
#define CFGADDR_MSUBSYS_ID1		0x6B

// BCD : EUI-64
#define CFGBASE_EUI64			0x61
#define CFGADDR_EUI64_0			0x44
#define CFGADDR_EUI64_1			0x45
#define CFGADDR_EUI64_2			0x46
#define CFGADDR_EUI64_3			0x47
#define CFGADDR_EUI64_4			0x48
#define CFGADDR_EUI64_5			0x49
#define CFGADDR_EUI64_6			0x4A
#define CFGADDR_EUI64_7			0x4B

// BCD : EUI-64 Mirror
#define CFGBASE_MEUI64			0x69
#define CFGADDR_MEUI64_0		0x80
#define CFGADDR_MEUI64_1		0x81
#define CFGADDR_MEUI64_2		0x82
#define CFGADDR_MEUI64_3		0x83
#define CFGADDR_MEUI64_4		0x84
#define CFGADDR_MEUI64_5		0x85
#define CFGADDR_MEUI64_6		0x86
#define CFGADDR_MEUI64_7		0x87

#define ACD_HEADER_SIZE			4
#define ACD_MULTIPURPOSE		0x0001
#define ACD_SROMINFO			0x0002


//------------------------------------------------------------------------------------
// File
//------------------------------------------------------------------------------------

#define DEF_COMMENT_MART		LITERAL(';')	// comment

// Default SubSystem Vendor ID, SubSystem ID
#define DEFAULT_SUBSYSTEM_VENDOR_ID		0xFFFF
#define DEFAULT_SUBSYSTEM_ID			0xFFFF

// Vendor Specific Configuration Data ID
#define CFG_SECTION_SUBSYSTEM_VENDOR_ID 0x00040000UL
#define CFG_SECTION_SUBSYSTEM_ID        0x00020000UL

#define CFG_SECTION_PHYSET1_ID          0x01000000UL
#define CFG_SECTION_PHYSET2_ID          0x02000000UL
#if 0
#define CFG_SECTION_PHYSET3_ID          0x04000000UL
#define CFG_SECTION_CHIPSET_ID          0x08000000UL
#endif
/* bit23:16 -> PCI Reg Adder, bit7:0 -> set bit position(lower bit) */
// PCICONF0DC
#define CFG_SECTION_USB3TXPHY_P1_ID     0x00DC0000UL
#define CFG_SECTION_USB3TXPHY_P2_ID     0x00DC0010UL

#define SHIFT_BIT_PCICONF_PORT          16

#define CFG_SECTION_USB3TXPHY_PARAM_0   0x0000UL
#define CFG_SECTION_USB3TXPHY_PARAM_1   0x0001UL
#define CFG_SECTION_USB3TXPHY_PARAM_2   0x0010UL
#define CFG_SECTION_USB3TXPHY_PARAM_3   0x0100UL
#define CFG_SECTION_USB3TXPHY_PARAM_4   0x1000UL

#define MASK_BIT_USB3TXPHY_PORT         0x0000FFFFUL

// PCICONF0E0
#define CFG_SECTION_USB3TXPHY_P3_ID     0x00DC0015UL
#define CFG_SECTION_USB3TXPHY_P4_ID     0x00DC000BUL

// PCICONF0E4
#define CFG_SECTION_BC_MODE_P1_ID       0x00E40000UL
#define CFG_SECTION_BC_MODE_P2_ID       0x00E40004UL
#define CFG_SECTION_BC_MODE_P3_ID       0x00E40008UL
#define CFG_SECTION_BC_MODE_P4_ID       0x00E4000CUL
#define CFG_SECTION_TRTFCTL_P1_ID       0x00E40010UL
#define CFG_SECTION_TRTFCTL_P2_ID       0x00E40012UL
#define CFG_SECTION_TRTFCTL_P3_ID       0x00E40014UL
#define CFG_SECTION_TRTFCTL_P4_ID       0x00E40016UL
#define CFG_SECTION_IMPCTL_ID           0x00E40018UL

#define MASK_BIT_BC_MODE_P1             0x0000000FUL
#define MASK_BIT_BC_MODE_P2             0x000000F0UL
#define MASK_BIT_BC_MODE_P3             0x00000F00UL
#define MASK_BIT_BC_MODE_P4             0x0000F000UL
#define MASK_BIT_TRTFCTL_P1             0x00030000UL
#define MASK_BIT_TRTFCTL_P1_202         0x00050000UL
#define MASK_BIT_TRTFCTL_P2             0x000C0000UL
#define MASK_BIT_TRTFCTL_P2_202         0x000A0000UL
#define MASK_BIT_TRTFCTL_P3             0x00300000UL
#define MASK_BIT_TRTFCTL_P4             0x00C00000UL
#define MASK_BIT_IMPCTL                 0x03000000UL
#define MASK_BIT_D202_SETDATA           0x5

#define MAX_PARAM_BC_MODE               7
#define MAX_PARAM_TRTFCTL               3
#define MAX_PARAM_IMPCTL                3

#define SHIFT_BIT_BC_MODE_P2             4
#define SHIFT_BIT_BC_MODE_P3             8
#define SHIFT_BIT_BC_MODE_P4            12
#define SHIFT_BIT_TRTFCTL_P1            16
#define SHIFT_BIT_TRTFCTL_D202_P2       17
#define SHIFT_BIT_TRTFCTL_P2            18
#define SHIFT_BIT_TRTFCTL_P3            20
#define SHIFT_BIT_TRTFCTL_P4            22
#define SHIFT_BIT_IMPCTL                24

// PCICONF0E8
#define CFG_SECTION_TX_CMODE_P1_ID      0x00E80000UL
#define CFG_SECTION_TX_CMODE_P2_ID      0x00E80001UL
#define CFG_SECTION_TX_CMODE_P3_ID      0x00E80002UL
#define CFG_SECTION_TX_CMODE_P4_ID      0x00E80003UL
#define CFG_SECTION_NON_REMOVAL_P1_ID   0x00E80008UL
#define CFG_SECTION_NON_REMOVAL_P2_ID   0x00E80009UL
#define CFG_SECTION_NON_REMOVAL_P3_ID   0x00E8000AUL
#define CFG_SECTION_NON_REMOVAL_P4_ID   0x00E8000BUL
#define CFG_SECTION_PTPWR_CTRL_ID       0x00E80010UL
#define CFG_SECTION_DISABLE_PORT_CNT_ID 0x00E80011UL
#define CFG_SECTION_DISABLE_COMP_ID     0x00E80013UL  //2012/07 Bit 19 Disable Conmpletion Timeout
#define CFG_SECTION_Set_E8_Bit20        0x00E80014UL  //2012/07 Specific settingBit20
#define CFG_SECTION_Set_E8_Bit21        0x00E80015UL  //2012/07 Specific settingBit21
#define CFG_SECTION_Set_E8_Bit23_22     0x00E80016UL  //2012/07 Specific settingBit23:22
#define CFG_SECTION_PSEL_ID             0x00E80018UL
#define CFG_SECTION_CARDSEL_ID          0x00E80019UL
#define CFG_SECTION_AUXDET_ID           0x00E8001AUL
#define CFG_SECTION_CLKREQ_FORCE_ID     0x00E8001BUL
#define CFG_SECTION_SERIALNUM_CAP_EN_ID 0x00E8001CUL
#define CFG_SECTION_PCI_TXSWING_CTRL_ID 0x00E8001DUL
#define CFG_SECTION_OSC_THROUGH_MODE_ID 0x00E8001EUL
#define CFG_SECTION_FWUPDATE_ID         0x00E80020UL

#define MASK_BIT_TX_CMODE_P1            0x00000001UL
#define MASK_BIT_TX_CMODE_P2            0x00000002UL
#define MASK_BIT_TX_CMODE_P3            0x00000004UL
#define MASK_BIT_TX_CMODE_P4            0x00000008UL
#define MASK_BIT_NON_REMOVAL_P1         0x00000100UL
#define MASK_BIT_NON_REMOVAL_P2         0x00000200UL
#define MASK_BIT_NON_REMOVAL_P3         0x00000400UL
#define MASK_BIT_NON_REMOVAL_P4         0x00000800UL
#define MASK_BIT_PTPWR_CTRL             0x00010000UL
#define MASK_BIT_DISABLE_PORT_CNT       0x00060000UL
#define MASK_BIT_DISABLE_COMPLETION     0x00080000UL //2012/07 Disable Conmpletion Timeout
#define MASK_BIT_Set_E8_Bit20           0x00100000UL //2012/07 Speific setting 0xE8 Bit20
#define MASK_BIT_Set_E8_Bit21           0x00200000UL //2012/07 Speific setting 0xE8 Bit21
#define MASK_BIT_Set_E8_Bit23_22        0x00C00000UL //2012/07 Speific setting 0xE8 Bit23:22
#define MASK_BIT_PSEL                   0x01000000UL
#define MASK_BIT_CARDSEL                0x02000000UL
#define MASK_BIT_AUXDET                 0x04000000UL
#define MASK_BIT_CLKREQ_FORCE           0x08000000UL
#define MASK_BIT_SERIALNUM_CAP_EN       0x10000000UL
#define MASK_BIT_PCI_TXSWING_CTRL       0x20000000UL
#define MASK_BIT_OSC_THROUGH_MODE       0x40000000UL

#define MAX_PARAM_DISABLE_PORT_CNT       3
#define MAX_PARAM_DISABLE_PORT_CNT_202   1
#define MAX_PARAM_Set_E8_Bit23_22        3 //2012/07 Specific settingBit23:22

#define SHIFT_BIT_TX_CMODE_P2            1
#define SHIFT_BIT_TX_CMODE_P3            2
#define SHIFT_BIT_TX_CMODE_P4            3
#define SHIFT_BIT_TNON_REMOVAL_P1        8
#define SHIFT_BIT_TNON_REMOVAL_P2        9
#define SHIFT_BIT_TNON_REMOVAL_P3       10
#define SHIFT_BIT_TNON_REMOVAL_P4       11
#define SHIFT_BIT_PTPWR_CTRL            16
#define SHIFT_BIT_DISABLE_PORT_CTRL     17
#define SHIFT_BIT_DISABLE_COMPLETION    19 //2012/07 DisableCompletionTimeout
#define SHIFT_BIT_Set_E8_Bit20          20 //2012/07 Specific settingBit20
#define SHIFT_BIT_Set_E8_Bit21          21 //2012/07 Specific settingBit21
#define SHIFT_BIT_Set_E8_Bit23_22       22 //2012/07 Specific settingBit23:22
#define SHIFT_BIT_PSEL                  24
#define SHIFT_BIT_CARDSEL               25
#define SHIFT_BIT_AUXDET                26
#define SHIFT_BIT_CLKREQ_FORCE          27
#define SHIFT_BIT_SERIALNUM_CAP_EN      28
#define SHIFT_BIT_PCI_TXSWING_CTRL      29
#define SHIFT_BIT_OSC_THROUGH_MODE      30

#define CFG_SECTION_NON_ID              0xFFFFFFFFUL

#define CFG_D201_DEFAULT_VALUE_E4       0x00550000UL
#define CFG_D201_DEFAULT_VALUE_E8       0x05010000UL
#define CFG_D202_DEFAULT_VALUE_E4       0x00030000UL
#define CFG_D202_DEFAULT_VALUE_E8       0x05010000UL

#define SET_CFG_TRTFCTL_D202_0          0UL
#define SET_CFG_TRTFCTL_D202_1          1UL
#define SET_CFG_TRTFCTL_D202_2          4UL
#define SET_CFG_TRTFCTL_D202_3          5UL

#define PHY_SETTING			1
#define PHY_NO_SETTING		0

#define BASE_UCHAR			1
#define BASE_UINT			2
#define BASE_ULONG			3

#define DISABLE_PORT_CNT_INVALID_AREA_D202 0x00040000UL
// Vendor Specific Configuration Data 1block size
#define VSC_DATA_BLOCK_SIZE_1		1
#define VSC_DATA_BLOCK_SIZE_2		2
#define VSC_DATA_BLOCK_SIZE_4		4
#define VSC_DATA_BLOCK_SIZE_8		8

#define VSC_DATA_SHIFT_BIT			8

// Mask of Vendor Specific Configuration Data Block
#define VSC_DATA_BLOCK_MASK		0xFF


// Vendor Specific Configuration address
#define VSC_DATA_ADDR_SUBSYSTEM_VID		0x0
#define VSC_DATA_ADDR_SUBSYSTEM_ID		0x2
#define VSC_DATA_ADDR_SERIAL_NUM1		0x4
#define VSC_DATA_ADDR_SERIAL_NUM2		0x8
#define VSC_DATA_ADDR_PHY_SETTING1		0xC
#define VSC_DATA_ADDR_PHY_SETTING2		0x10
#define VSC_DATA_ADDR_PHY_SETTING3		0x14
#define VSC_DATA_ADDR_CHIP_SETTING		0x18

// CRC
#define CRC_16		0x0

#define SROM_WRITE_SIZE_64K				0x10000UL


#define SROM_ERASE_BLOCK_SIZE_4K		 0x1000UL
#define SROM_ERASE_BLOCK_SIZE_32K		 0x8000UL
#define SROM_ERASE_BLOCK_SIZE_64K		0x10000UL
#define SROM_ERASE_BLOCK_SIZE_128K		0x20000UL

#define SROM_ERASE_BLOCK_MASK			   0x30
#define SROM_ERASE_BLOCK_SHIFT_BIT		   4

//------------------------------------------------------------------------------------
// xHCI
//------------------------------------------------------------------------------------

#define FW_UPDATER_FOOTER_SIZE		(16*1024) // 16KB

//********************************************************************************//
//	struct
//********************************************************************************//

#ifndef __linux__
#pragma pack(1)
#endif//__linux__

// Serial ROM Image Header
typedef struct _SROMIMAGE_HEADER {
	USHORT		StartUpCode;	// Start up Code / 
	USHORT		BCDAddr;		// Boot Configration Data Offset
	USHORT		BCDSize;		// Boot Configration Data Size
	USHORT		FWIDAddr;		// F/W Image Data Offset
	USHORT		FWIDSize;		// F/W Image Data Size
	USHORT		ACDAddr;		// Application COnfigration Data Offset
	USHORT		ACDSize;		// Application COnfigration Data Size
} SROMIMAGE_HEADER,*PSROMIMAGE_HEADER;

// Extended Serial ROM Image Header
typedef struct _SROMIMAGE_HEADER_EX {
	USHORT		StartUpCode;	// Start up Code / 
	USHORT		BCDAddr;		// Boot Configration Data Offset
	USHORT		BCDSize;		// Boot Configration Data Size
	USHORT		FWIDAddr;		// F/W Image Data Offset
	USHORT		FWIDSize;		// F/W Image Data Size
	USHORT		ACDAddr;		// Application COnfigration Data Offset
	USHORT		ACDSize;		// Application COnfigration Data Size
	USHORT		ConfigVer;		// Config Version
} SROMIMAGE_HEADER_EX,*PSROMIMAGE_HEADER_EX;


#pragma pack()

//********************************************************************************//
//	macro
//********************************************************************************//

#define GET_PCIADDR(x)		((USHORT)(x & 0xFFFF))
#define MAKE_PCIADDR(x)		((ULONG)x & 0x0000FFFF)

//#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
//#define max(a, b)  (((a) < (b)) ? (b) : (a)) 


//********************************************************************************//
//	LINK list 
//********************************************************************************//


/*++

LINK list:

    Definitions for a double link list.

--*/

//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//
#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) \
    ((type *)((PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))
#endif


#ifndef InitializeListHead
//
//  VOID
//  InitializeListHead(
//      PLIST_ENTRY ListHead
//      );
//

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

//
//  PLIST_ENTRY
//  RemoveTailList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

//
//  VOID
//  InsertHeadList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }



BOOL IsNodeOnList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry);


#endif //InitializeListHead




#endif // #ifndef _COMDEF_H_
