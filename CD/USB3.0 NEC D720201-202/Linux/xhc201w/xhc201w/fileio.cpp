/*
	File Name : fileio.c
	
	file I/O 

	* Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//      2010-08-16  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "xhc201w.h"
#include "xhc201h.h"

#include "comutil.h"	// common utility


#include "fileio.h"

//********************************************************************************//
//	Global
//********************************************************************************//

// ROM Image file
FILE* g_ImgFile = NULL;

// FW file
FILE* g_FwFile = NULL;

//********************************************************************************//
//	Extern
//********************************************************************************//
extern int g_findDevFlag;

//********************************************************************************//
//	define
//********************************************************************************//

//********************************************************************************//
//	Function
//********************************************************************************//
//
// Remove Control Charactor
//
int ReplaceString(LPTSTR str)
{
	size_t len;
	int p;
	int i;
	
	len = STRLEN(str);
	p = 0;
	
	for(i=0;i<(int)len;i++){
	
		if(str[i] == DEF_COMMENT_MART){
			break;
		}

		if((ISCNTRL(str[i]) == 0) &&
		   (str[i] != LITERAL(' '))){
			if(i != p){
				str[p] = str[i];
			}
			p++;
		}
	}
	
	str[p] = LITERAL('\0');
	
	return p;
}


//
// Convert string to long
//
int Str2Val(LPTSTR pStr, int nBase, ULONG *pValue, int radix)
{
	LPTSTR endptr;
	ULONG max_value;
	
//	DebugTrace (LITERAL("%s\n"),pStr); // for debug

	switch (nBase) {

		case 1:
			max_value = UCHAR_MAX;
			break;

		case 2:
			max_value = UINT_MAX;
			break;

#ifdef _XHCI_D720201
		case 3:
			max_value = ULONG_MAX;
			break;
#endif
		case 4:
			max_value = 0;
			break;

		default:
			return -1;

	}

	*pValue = STRTOUL(pStr, &endptr, radix);

	if(0 == *pValue){
		// not convert
		if (pStr == endptr) {
			return -1;
		}

	}else if(max_value){
		// value too big
		if(max_value < *pValue){
			return -1;
		}
	}

	return 0;	
	
}



///////////////////////////////////////////////////////////////////////////////
//		FW Image File Function
///////////////////////////////////////////////////////////////////////////////

//
// ROM Image file open
// 
int OpenImage(LPCTSTR fname,long *flength)
{
//	int hFile;
	errno_t err;
	
	if(g_ImgFile){
		DebugTrace (LITERAL("Other ROM Image file already opened!\n"));
		return XHCSTS_MEMFILE_ERR;
	}
	
	err = FOPEN(&g_ImgFile, fname, LITERAL("rb"));

	if (err) {
		g_ImgFile = NULL;
		DebugTrace (LITERAL("ROM Image file open error: %s(rb) (%d)\n"), fname, err);
		return XHCSTS_MEMFILE_NOT_FOUND;
	}

	{
		struct _stat buf;
		if(FSTAT(FILENO(g_ImgFile), &buf) == 0) {
			*flength = buf.st_size;
		}
		else{
			*flength = 0;
		}
	}

	DebugTrace (LITERAL("ROM Image file size = %d\n"), *flength);

	return XHCSTS_SUCCESS;
}


//
// ROM Image file close
//
void CloseImage(void)
{
	if(g_ImgFile){
		FCLOSE (g_ImgFile);
		g_ImgFile = NULL;
	}
}


//
// ROM Image Read 
//
int ReadImage(long offset,UCHAR *pBuf, size_t size)
{
	if(!g_ImgFile){
		DebugTrace (LITERAL("ROM Image file not opened!\n"));
		return XHCSTS_MEMFILE_ERR;
	}	
	
	if(FSEEK(g_ImgFile,offset,SEEK_SET)){
		DebugTrace (LITERAL("ROM Image file seek error!\n"));
		return XHCSTS_MEMFILE_ERR;
	}
	
	return (int)FREAD(pBuf,1,size,g_ImgFile);
}



//
// Load Image
//
int LoadSROMImage(LPCTSTR fname,
				  long size,
				  UCHAR *pBuf,
				  ULONG *pFileSize,
				  ULONG *pLoadSize
				  )
{
	size_t readSize;
	long flen;
	int ret;

	ret = OpenImage(fname, &flen);

	if(ret != XHCSTS_SUCCESS){
		return ret;
	}

	*pFileSize = flen;

	readSize = min(size,flen);

	ret = ReadImage(0,pBuf,readSize);

	if(ret == (int)readSize){
		*pLoadSize = ret;
		ret = XHCSTS_SUCCESS;
	}
	else{
		ret = XHCSTS_MEMFILE_ERR; 
	}

	CloseImage();

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//		FW File Function
///////////////////////////////////////////////////////////////////////////////

//
// FW file open
// 
int OpenFwData(LPCTSTR fname,long *flength)
{
//	int hFile;
	errno_t err;
	
	if(g_FwFile){
		DebugTrace (LITERAL("Other FW file already opened!\n"));
		return XHCSTS_FWFILE_ERR;
	}
	
	err = FOPEN(&g_FwFile, fname, LITERAL("rb"));

	if (err) {
		g_FwFile = NULL;
		DebugTrace (LITERAL("FW file open error: %s(rb) (%d)\n"), fname, err);
		return XHCSTS_FWFILE_NOT_FOUND;
	}

	{
		struct _stat buf;
		if(FSTAT(FILENO(g_FwFile), &buf) == 0) {
			*flength = buf.st_size;
		}
		else{
			*flength = 0;
		}
	}

	DebugTrace (LITERAL("FW file size = %d\n"), *flength);

	// size check
	if(*flength <= FWID_LD_SIZE){
		return XHCSTS_FWFILE_ERR;
	}


	return XHCSTS_SUCCESS;
}


//
// FW file close
//
void CloseFwData(void)
{
	if(g_FwFile){
		FCLOSE (g_FwFile);
		g_FwFile = NULL;
	}
}


//
// FW Read 
//
int ReadFwData(long offset,UCHAR *pBuf, size_t size)
{
	if(!g_FwFile){
		DebugTrace (LITERAL("FW file not opened!\n"));
		return XHCSTS_FWFILE_ERR;
	}	
	
	if(FSEEK(g_FwFile,offset,SEEK_SET)){
		DebugTrace (LITERAL("FW file seek error!\n"));
		return XHCSTS_FWFILE_ERR;
	}
	
	return (int)FREAD(pBuf,1,size,g_FwFile);
}



//
// Load FW
//
int LoadFwData(LPCTSTR fname,
			   long size,
			   UCHAR *pBuf,
			   ULONG *pFileSize,
			   ULONG *pLoadSize
			  )
{
	size_t readSize;
	long flen;
	int ret;

	ret = OpenFwData(fname, &flen);

	if(ret != XHCSTS_SUCCESS){
		return ret;
	}
	
	*pFileSize = flen;

	readSize = min(size,flen);

	ret = ReadFwData(0,pBuf,readSize);

	if(ret == (int)readSize){
		*pLoadSize = ret;
		ret = XHCSTS_SUCCESS;
	}
	else{
		ret = XHCSTS_FWFILE_ERR; 
	}

	CloseFwData();

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//		Config File Function
///////////////////////////////////////////////////////////////////////////////

//
// Read & analyze Config Data
//

///////////////////////////////////////////////////////////////////////////////
//		EUI-64 File Function
///////////////////////////////////////////////////////////////////////////////

//
// Read EUI-64
//
int ReadEui64(LPCTSTR euiname, UCHAR* pEui64, UINT index)
{
	FILE *fp;
	TCHAR line[LINE_BUFFER_SIZE];
	TCHAR *token, *context;
	LPTSTR basestr;
	TCHAR sep[2];
	ULONG value;
	int i;
	UINT count;
	int line_count;
	errno_t err;
	
	// Parameter check
	if((!pEui64) ||
	   (index < 1))
	{
		return XHCSTS_ERROR;
	}	
	
	
	// file open
	err = FOPEN (&fp, euiname, LITERAL("rt"));

	if(err) {
		DebugTrace (LITERAL("EUI-64 file open error: %s(rb) (%d)\n"), euiname, err);
		return XHCSTS_EUIFILE_NOT_FOUND;
	}
	
	sep[1] = LITERAL('\0');
	count = 0;
	line_count = 0;
	
	while(count != index){
		if(!FGETS(line, LINE_BUFFER_SIZE, fp)){
			break;
		}
		
		ReplaceString(line);
		
		
		if((line[0] == LITERAL('\0')) || (line[0] == DEF_COMMENT_MART)){
			line_count++;
			continue;
		}
		
		basestr = line;
		sep[0] = LITERAL('-');
		
		for(i=0;i<8;i++){
		
			if(i == 7){
				sep[0] = LITERAL('\0');	// last separater = NULL
			}

			token = STRTOK(basestr, sep, &context);
				
			if(!token){
				DebugTrace (LITERAL("EUI-64 data is unknown format! (line = %d)\n"),line_count);
				break;
			}		
			
			if(Str2Val(token, 1, &value, 16) != 0){
				DebugTrace (LITERAL("EUI-64 data is unknown format! (line = %d)\n"),line_count);
				break;
			}
			
			pEui64[i] = (UCHAR)value;
		
			basestr = NULL;
		}
		
		if(i != 8){
			break;
		}
		
		count++;
		line_count++;
	}
	
	FCLOSE(fp);
	
	if(count != index){
		return XHCSTS_EUIFILE_ERR;
	}

	return XHCSTS_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
//		Config File Function(d720201)
///////////////////////////////////////////////////////////////////////////////

//
// Read & analyze Config Data
//
int ReadVendorSpecificConfigInfo(LPCTSTR cfgname, 
								 USHORT *pSubSysID,
								 USHORT *pSubSysVenID,
								 PXHC_CFG_SETTING_INFO pconfigSetting,
								 BOOL *fwupdate)
{
	FILE *fp;
	TCHAR line[LINE_BUFFER_SIZE];
	unsigned long value;
	int bSSID,bSSVID;
	int fEof;
	int line_count;
	unsigned long section;
	errno_t err;

	if((!pSubSysVenID) || (!pSubSysID))
	{
		return XHCSTS_ERROR;
	}

	line_count = 1;
	fEof   = 0;
	bSSID  = 0;
	bSSVID = 0;
	section = CFG_SECTION_NON_ID;
	*pSubSysID    = DEFAULT_SUBSYSTEM_ID;
	*pSubSysVenID = DEFAULT_SUBSYSTEM_VENDOR_ID;

//	memset(pconfigSetting, 0x0, sizeof(PXHC_CFG_SETTING_INFO));

	// file open
	err = FOPEN (&fp, cfgname, LITERAL("rt"));

	if(err){
		DebugTrace (LITERAL("Config file open error: %s(rt) (%d)\n"), cfgname, err);
		return XHCSTS_CFGFILE_NOT_FOUND;
	}

	while(1){
		if(!FGETS(line,LINE_BUFFER_SIZE,fp)){
			fEof = FEOF(fp);
			break;
		}

		ReplaceString(line);
		if((line[0] == LITERAL('\0')) || (line[0] == DEF_COMMENT_MART)){
			line_count++;
			continue;
		}

		// SubSystemVendorID
		if(STRICMP(line, LITERAL("[SubSystemVendorID]"), 19) == 0){
			section = CFG_SECTION_SUBSYSTEM_VENDOR_ID;
		}
		else if(STRICMP(line, LITERAL("[SubSystemID]"), 13) == 0){
			section = CFG_SECTION_SUBSYSTEM_ID;
		}

		else if(STRICMP(line, LITERAL("[USB3TXPHYSettingPort1]"), 23) == 0){
			section = CFG_SECTION_USB3TXPHY_P1_ID;
		}
		else if(STRICMP(line, LITERAL("[USB3TXPHYSettingPort2]"), 23) == 0){
			section = CFG_SECTION_USB3TXPHY_P2_ID;
		}
		else if(STRICMP(line, LITERAL("[USB3TXPHYSettingPort3]"), 23) == 0){
			section = CFG_SECTION_USB3TXPHY_P3_ID;
		}
		else if(STRICMP(line, LITERAL("[USB3TXPHYSettingPort4]"), 23) == 0){
			section = CFG_SECTION_USB3TXPHY_P4_ID;
		}
		else if(STRICMP(line, LITERAL("[BatteryChargeModePort1]"), 24) == 0){
			section = CFG_SECTION_BC_MODE_P1_ID;
		}
		else if(STRICMP(line, LITERAL("[BatteryChargeModePort2]"), 24) == 0){
			section = CFG_SECTION_BC_MODE_P2_ID;
		}
		else if(STRICMP(line, LITERAL("[BatteryChargeModePort3]"), 24) == 0){
			section = CFG_SECTION_BC_MODE_P3_ID;
		}
		else if(STRICMP(line, LITERAL("[BatteryChargeModePort4]"), 24) == 0){
			section = CFG_SECTION_BC_MODE_P4_ID;
		}
		else if(STRICMP(line, LITERAL("[TRTFCTLU2Dx1]"), 14) == 0){
			section = CFG_SECTION_TRTFCTL_P1_ID;
		}
		else if(STRICMP(line, LITERAL("[TRTFCTLU2Dx2]"), 14) == 0){
			section = CFG_SECTION_TRTFCTL_P2_ID;
		}
		else if(STRICMP(line, LITERAL("[TRTFCTLU2Dx3]"), 14) == 0){
			section = CFG_SECTION_TRTFCTL_P3_ID;
		}
		else if(STRICMP(line, LITERAL("[TRTFCTLU2Dx4]"), 14) == 0){
			section = CFG_SECTION_TRTFCTL_P4_ID;
		}
		else if(STRICMP(line, LITERAL("[IMPCTL]"), 8) == 0){
			section = CFG_SECTION_IMPCTL_ID;
		}
		else if(STRICMP(line, LITERAL("[EnableU3Tx1CustomMode]"), 23) == 0){
			section = CFG_SECTION_TX_CMODE_P1_ID;
		}
		else if(STRICMP(line, LITERAL("[EnableU3Tx2CustomMode]"), 23) == 0){
			section = CFG_SECTION_TX_CMODE_P2_ID;
		}
		else if(STRICMP(line, LITERAL("[EnableU3Tx3CustomMode]"), 23) == 0){
			section = CFG_SECTION_TX_CMODE_P3_ID;
		}
		else if(STRICMP(line, LITERAL("[EnableU3Tx4CustomMode]"), 23) == 0){
			section = CFG_SECTION_TX_CMODE_P4_ID;
		}
		else if(STRICMP(line, LITERAL("[EnableDeviceNonRemovalPort1]"), 29) == 0){
			section = CFG_SECTION_NON_REMOVAL_P1_ID;
		}
		else if(STRICMP(line, LITERAL("[EnableDeviceNonRemovalPort2]"), 29) == 0){
			section = CFG_SECTION_NON_REMOVAL_P2_ID;
		}
		else if(STRICMP(line, LITERAL("[EnableDeviceNonRemovalPort3]"), 29) == 0){
			section = CFG_SECTION_NON_REMOVAL_P3_ID;
		}
		else if(STRICMP(line, LITERAL("[EnableDeviceNonRemovalPort4]"), 29) == 0){
			section = CFG_SECTION_NON_REMOVAL_P4_ID;
		}
		else if(STRICMP(line, LITERAL("[UsePPON]"), 9) == 0){
			section = CFG_SECTION_PTPWR_CTRL_ID;
		}
		else if(STRICMP(line, LITERAL("[DisablePortCount]"), 18) == 0){
			section = CFG_SECTION_DISABLE_PORT_CNT_ID;
		}
		// Added the new section 2012/07
		else if(STRICMP(line, LITERAL("[DisableCompletionTimeout]"), 26) == 0){
			section = CFG_SECTION_DISABLE_COMP_ID;
		}
		//
		// Added the new section 2012/07
		else if(STRICMP(line, LITERAL("[SetOffset0xE8_Bit20]"), 21) == 0){
			section = CFG_SECTION_Set_E8_Bit20;
		}
		//
		// Added the new section 2012/07
		else if(STRICMP(line, LITERAL("[SetOffset0xE8_Bit21]"), 21) == 0){
			section = CFG_SECTION_Set_E8_Bit21;
		}
		//
		// Added the new section 2012/07
		else if(STRICMP(line, LITERAL("[SetOffset0xE8_Bit23:22]"), 24) == 0){
			section = CFG_SECTION_Set_E8_Bit23_22;
		}
		//
		else if(STRICMP(line, LITERAL("[PSEL]"), 6) == 0){
			section = CFG_SECTION_PSEL_ID;
		}
		else if(STRICMP(line, LITERAL("[CARDSEL]"), 9) == 0){
			section = CFG_SECTION_CARDSEL_ID;
		}
		else if(STRICMP(line, LITERAL("[AUXDET]"), 8) == 0){
			section = CFG_SECTION_AUXDET_ID;
		}
		else if(STRICMP(line, LITERAL("[EnableClockRequest]"), 20) == 0){
			section = CFG_SECTION_CLKREQ_FORCE_ID;
		}
		else if(STRICMP(line, LITERAL("[SerialNumCapEnable]"), 20) == 0){
			section = CFG_SECTION_SERIALNUM_CAP_EN_ID;
		}
		else if(STRICMP(line, LITERAL("[PCIeTxSwingControl]"), 20) == 0){
			section = CFG_SECTION_PCI_TXSWING_CTRL_ID;
		}
		else if(STRICMP(line, LITERAL("[EnableOSCThroughMode]"), 22) == 0){
			section = CFG_SECTION_OSC_THROUGH_MODE_ID;
		}
		else if(STRICMP(line, LITERAL("[FWUpdateProperty]"), 18) == 0){
			section = CFG_SECTION_FWUPDATE_ID;
		}
		else{
			if(section == CFG_SECTION_SUBSYSTEM_VENDOR_ID){
				if(Str2Val(line, BASE_UINT, &value, 16) != 0){
					DebugTrace (LITERAL("SubSytemVendor ID is unknown format! (line = %d)\n"),line_count);
					break;
				}

				*pSubSysVenID = (USHORT)value;
				bSSVID  = 1;
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_SUBSYSTEM_ID){
				if(Str2Val(line, BASE_UINT, &value, 16) != 0){
					DebugTrace (LITERAL("SubSytem ID is unknown format! (line = %d)\n"),line_count);
					break;
				}

				*pSubSysID = (USHORT)value;
				bSSID   = 1;
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_USB3TXPHY_P1_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[USB3TXPHYSettingPort1] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				pconfigSetting->phySetting1 &= ~MASK_BIT_USB3TXPHY_PORT;
				if(value == 1){
					pconfigSetting->phySetting1 |= CFG_SECTION_USB3TXPHY_PARAM_1;
				}
				else if(value == 2){
					pconfigSetting->phySetting1 |= CFG_SECTION_USB3TXPHY_PARAM_2;
				}
				else if(value == 3){
					pconfigSetting->phySetting1 |= CFG_SECTION_USB3TXPHY_PARAM_3;
				}
				else if(value == 4){
					pconfigSetting->phySetting1 |= CFG_SECTION_USB3TXPHY_PARAM_4;
				}
				else{
					if(value != 0){
						DebugTrace (LITERAL("[USB3TXPHYSettingPort1]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_USB3TXPHY_P2_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[USB3TXPHYSettingPort2] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				pconfigSetting->phySetting1 &= ~(MASK_BIT_USB3TXPHY_PORT << SHIFT_BIT_PCICONF_PORT);
				if(value == 1){
					pconfigSetting->phySetting1 |= (CFG_SECTION_USB3TXPHY_PARAM_1 << SHIFT_BIT_PCICONF_PORT);
				}
				else if(value == 2){
					pconfigSetting->phySetting1 |= (CFG_SECTION_USB3TXPHY_PARAM_2 << SHIFT_BIT_PCICONF_PORT);
				}
				else if(value == 3){
					pconfigSetting->phySetting1 |= (CFG_SECTION_USB3TXPHY_PARAM_3 << SHIFT_BIT_PCICONF_PORT);
				}
				else if(value == 4){
					pconfigSetting->phySetting1 |= (CFG_SECTION_USB3TXPHY_PARAM_4 << SHIFT_BIT_PCICONF_PORT);
				}
				else{
					if(value != 0){
						DebugTrace (LITERAL("[USB3TXPHYSettingPort2]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}

				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_USB3TXPHY_P3_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[USB3TXPHYSettingPort3] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				pconfigSetting->phySetting2 &= ~MASK_BIT_USB3TXPHY_PORT;
				if(value == 1){
					pconfigSetting->phySetting2 |= CFG_SECTION_USB3TXPHY_PARAM_1;
				}
				else if(value == 2){
					pconfigSetting->phySetting2 |= CFG_SECTION_USB3TXPHY_PARAM_2;
				}
				else if(value == 3){
					pconfigSetting->phySetting2 |= CFG_SECTION_USB3TXPHY_PARAM_3;
				}
				else if(value == 4){
					pconfigSetting->phySetting2 |= CFG_SECTION_USB3TXPHY_PARAM_4;
				}
				else{
					if(value != 0){
						DebugTrace (LITERAL("[USB3TXPHYSettingPort3]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}

				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_USB3TXPHY_P4_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[USB3TXPHYSettingPort4] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				pconfigSetting->phySetting2 &= ~(MASK_BIT_USB3TXPHY_PORT << SHIFT_BIT_PCICONF_PORT);
				if(value == 1){
					pconfigSetting->phySetting2 |= (CFG_SECTION_USB3TXPHY_PARAM_1 << SHIFT_BIT_PCICONF_PORT);
				}
				else if(value == 2){
					pconfigSetting->phySetting2 |= (CFG_SECTION_USB3TXPHY_PARAM_2 << SHIFT_BIT_PCICONF_PORT);
				}
				else if(value == 3){
					pconfigSetting->phySetting2 |= (CFG_SECTION_USB3TXPHY_PARAM_3 << SHIFT_BIT_PCICONF_PORT);
				}
				else if(value == 4){
					pconfigSetting->phySetting2 |= (CFG_SECTION_USB3TXPHY_PARAM_4 << SHIFT_BIT_PCICONF_PORT);
				}
				else{
					if(value != 0){
						DebugTrace (LITERAL("[USB3TXPHYSettingPort4]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}

				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_BC_MODE_P1_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[BatteryChargeModePort1] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(value <= MAX_PARAM_BC_MODE){
					pconfigSetting->phySetting3 &= ~MASK_BIT_BC_MODE_P1;
					pconfigSetting->phySetting3 |= value;
				}
				else{
					DebugTrace (LITERAL("[BatteryChargeModePort1]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_BC_MODE_P2_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[BatteryChargeModePort2] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(value <= MAX_PARAM_BC_MODE){
					pconfigSetting->phySetting3 &= ~MASK_BIT_BC_MODE_P2;
					pconfigSetting->phySetting3 |= (value << SHIFT_BIT_BC_MODE_P2);
				}
				else{
					DebugTrace (LITERAL("[BatteryChargeModePort2]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_BC_MODE_P3_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[BatteryChargeModePort3] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(g_findDevFlag == DEVICE_ID_TYPE_D201){
					if(value <= MAX_PARAM_BC_MODE){
						pconfigSetting->phySetting3 &= ~MASK_BIT_BC_MODE_P3;
						pconfigSetting->phySetting3 |= (value << SHIFT_BIT_BC_MODE_P3);
					}
					else{
						DebugTrace (LITERAL("[BatteryChargeModePort3]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_BC_MODE_P4_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[BatteryChargeModePort4] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(g_findDevFlag == DEVICE_ID_TYPE_D201){
					if(value <= MAX_PARAM_BC_MODE){
						pconfigSetting->phySetting3 &= ~MASK_BIT_BC_MODE_P4;
						pconfigSetting->phySetting3 |= (value << SHIFT_BIT_BC_MODE_P4);
					}
					else{
						DebugTrace (LITERAL("[BatteryChargeModePort4]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_TRTFCTL_P1_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[TRTFCTLU2Dx1] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(value <= MAX_PARAM_TRTFCTL){
					if(g_findDevFlag == DEVICE_ID_TYPE_D201){
						pconfigSetting->phySetting3 &= ~MASK_BIT_TRTFCTL_P1;
						pconfigSetting->phySetting3 |= (value << SHIFT_BIT_TRTFCTL_P1);
					}
					else{
						pconfigSetting->phySetting3 &= ~MASK_BIT_TRTFCTL_P1_202;
						switch(value){
							case 0:
								pconfigSetting->phySetting3 |= (SET_CFG_TRTFCTL_D202_0 << SHIFT_BIT_TRTFCTL_P1);
								break;
							case 1:
								pconfigSetting->phySetting3 |= (SET_CFG_TRTFCTL_D202_1 << SHIFT_BIT_TRTFCTL_P1);
								break;
							case 2:
								pconfigSetting->phySetting3 |= (SET_CFG_TRTFCTL_D202_2 << SHIFT_BIT_TRTFCTL_P1);
								break;
							case 3:
								pconfigSetting->phySetting3 |= (SET_CFG_TRTFCTL_D202_3 << SHIFT_BIT_TRTFCTL_P1);
								break;
							default:
								break;
						}
					}
				}
				else{
					DebugTrace (LITERAL("[TRTFCTLU2Dx1]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_TRTFCTL_P2_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[TRTFCTLU2Dx2] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(value <= MAX_PARAM_TRTFCTL){
					if(g_findDevFlag == DEVICE_ID_TYPE_D201){
						pconfigSetting->phySetting3 &= ~MASK_BIT_TRTFCTL_P2;
						pconfigSetting->phySetting3 |= (value << SHIFT_BIT_TRTFCTL_P2);
					}
					else{
						pconfigSetting->phySetting3 &= ~MASK_BIT_TRTFCTL_P2_202;
						switch(value){
							case 0:
								pconfigSetting->phySetting3 |= (SET_CFG_TRTFCTL_D202_0 << SHIFT_BIT_TRTFCTL_D202_P2);
								break;
							case 1:
								pconfigSetting->phySetting3 |= (SET_CFG_TRTFCTL_D202_1 << SHIFT_BIT_TRTFCTL_D202_P2);
								break;
							case 2:
								pconfigSetting->phySetting3 |= (SET_CFG_TRTFCTL_D202_2 << SHIFT_BIT_TRTFCTL_D202_P2);
								break;
							case 3:
								pconfigSetting->phySetting3 |= (SET_CFG_TRTFCTL_D202_3 << SHIFT_BIT_TRTFCTL_D202_P2);
								break;
							default:
								break;
						}
					}
				}
				else{
					DebugTrace (LITERAL("[TRTFCTLU2Dx2]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_TRTFCTL_P3_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[TRTFCTLU2Dx3] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(g_findDevFlag == DEVICE_ID_TYPE_D201){
					if(value <= MAX_PARAM_TRTFCTL){
						pconfigSetting->phySetting3 &= ~MASK_BIT_TRTFCTL_P3;
						pconfigSetting->phySetting3 |= (value << SHIFT_BIT_TRTFCTL_P3);
					}
					else{
						DebugTrace (LITERAL("[TRTFCTLU2Dx3]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_TRTFCTL_P4_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[TRTFCTLU2Dx4] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(g_findDevFlag == DEVICE_ID_TYPE_D201){
					if(value <= MAX_PARAM_TRTFCTL){
						pconfigSetting->phySetting3 &= ~MASK_BIT_TRTFCTL_P4;
						pconfigSetting->phySetting3 |= (value << SHIFT_BIT_TRTFCTL_P4);
					}
					else{
						DebugTrace (LITERAL("[TRTFCTLU2Dx4]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_IMPCTL_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[IMPCTL] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(value <= MAX_PARAM_IMPCTL){
					pconfigSetting->phySetting3 &= ~MASK_BIT_IMPCTL;
					pconfigSetting->phySetting3 |= (value << SHIFT_BIT_IMPCTL);
				}
				else{
					DebugTrace (LITERAL("[IMPCTL]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_TX_CMODE_P1_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[EnableU3Tx1CustomMode] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_TX_CMODE_P1;
					pconfigSetting->chipSetting |= value;
				}
				else{
					DebugTrace (LITERAL("[EnableU3Tx1CustomMode]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_TX_CMODE_P2_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[EnableU3Tx2CustomMode] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_TX_CMODE_P2;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_TX_CMODE_P2);
				}
				else{
					DebugTrace (LITERAL("[EnableU3Tx2CustomMode]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_TX_CMODE_P3_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[EnableU3Tx3CustomMode] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(g_findDevFlag == DEVICE_ID_TYPE_D201){
					if((value == 0) || (value == 1)){
						pconfigSetting->chipSetting &= ~MASK_BIT_TX_CMODE_P3;
						pconfigSetting->chipSetting |= (value << SHIFT_BIT_TX_CMODE_P3);
					}
					else{
						DebugTrace (LITERAL("[EnableU3Tx3CustomMode]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_TX_CMODE_P4_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[EnableU3Tx4CustomMode] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(g_findDevFlag == DEVICE_ID_TYPE_D201){
					if((value == 0) || (value == 1)){
						pconfigSetting->chipSetting &= ~MASK_BIT_TX_CMODE_P4;
						pconfigSetting->chipSetting |= (value << SHIFT_BIT_TX_CMODE_P4);
					}
					else{
						DebugTrace (LITERAL("[EnableU3Tx4CustomMode]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_NON_REMOVAL_P1_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[EnableDeviceNonRemovalPort1] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_NON_REMOVAL_P1;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_TNON_REMOVAL_P1);
				}
				else{
					DebugTrace (LITERAL("[EnableDeviceNonRemovalPort1]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_NON_REMOVAL_P2_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[EnableDeviceNonRemovalPort2] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_NON_REMOVAL_P2;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_TNON_REMOVAL_P2);
				}
				else{
					DebugTrace (LITERAL("[EnableDeviceNonRemovalPort2]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_NON_REMOVAL_P3_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[EnableDeviceNonRemovalPort3] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(g_findDevFlag == DEVICE_ID_TYPE_D201){
					if((value == 0) || (value == 1)){
						pconfigSetting->chipSetting &= ~MASK_BIT_NON_REMOVAL_P3;
						pconfigSetting->chipSetting |= (value << SHIFT_BIT_TNON_REMOVAL_P3);
					}
					else{
						DebugTrace (LITERAL("[EnableDeviceNonRemovalPort3]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_NON_REMOVAL_P4_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[EnableDeviceNonRemovalPort4] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(g_findDevFlag == DEVICE_ID_TYPE_D201){
					if((value == 0) || (value == 1)){
						pconfigSetting->chipSetting &= ~MASK_BIT_NON_REMOVAL_P4;
						pconfigSetting->chipSetting |= (value << SHIFT_BIT_TNON_REMOVAL_P4);
					}
					else{
						DebugTrace (LITERAL("[EnableDeviceNonRemovalPort4]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_PTPWR_CTRL_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[UsePPON] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_PTPWR_CTRL;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_PTPWR_CTRL);
				}
				else{
					DebugTrace (LITERAL("[UsePPON]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_DISABLE_PORT_CNT_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[DisablePortCount] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(g_findDevFlag == DEVICE_ID_TYPE_D201){
					if(value <= MAX_PARAM_DISABLE_PORT_CNT){
						pconfigSetting->chipSetting &= ~MASK_BIT_DISABLE_PORT_CNT;
						pconfigSetting->chipSetting |= (value << SHIFT_BIT_DISABLE_PORT_CTRL);
					}
					else{
						DebugTrace (LITERAL("[DisablePortCount]value is Outside set range! (value = %d)\n"), value);
						break;
					}
				}
				else{
					if(value <= MAX_PARAM_DISABLE_PORT_CNT_202){
						pconfigSetting->chipSetting &= ~MASK_BIT_DISABLE_PORT_CNT;
						pconfigSetting->chipSetting |= (value << SHIFT_BIT_DISABLE_PORT_CTRL);
					}
					else{
						if(value > MAX_PARAM_DISABLE_PORT_CNT){
							DebugTrace (LITERAL("[DisablePortCount]value is Outside set range! (value = %d)\n"), value);
							break;
						}
					}
				}
				section = CFG_SECTION_NON_ID;
			}
			//Added the new section 2012/07
			else if(section == CFG_SECTION_DISABLE_COMP_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[DisableCompletionTimeout] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_DISABLE_COMPLETION;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_DISABLE_COMPLETION);
				}
				else{
					DebugTrace (LITERAL("[DisableCompletionTimeout] value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			//
			//Added the new section 2012/07
			else if(section == CFG_SECTION_Set_E8_Bit20){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[SetOffset0xE8_Bit20] is unknown format! (line = %d)\n"),line_count);
					break;
				}
				
				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_Set_E8_Bit20;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_Set_E8_Bit20);
				}
				else{
					DebugTrace (LITERAL("[SetOffset0xE8_Bit20] value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			//
			//Added the new section 2012/07
			else if(section == CFG_SECTION_Set_E8_Bit21){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[SetOffset0xE8_Bit21] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_Set_E8_Bit21;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_Set_E8_Bit21);
				}
				else{
					DebugTrace (LITERAL("[SetOffset0xE8_Bit21] value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			//
			//Added the new section 2012/07
			else if(section == CFG_SECTION_Set_E8_Bit23_22){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[SetOffset0xE8_Bit23:22] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(value <= MAX_PARAM_Set_E8_Bit23_22){
					pconfigSetting->chipSetting &= ~MASK_BIT_Set_E8_Bit23_22;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_Set_E8_Bit23_22);
				}
				else{
					DebugTrace (LITERAL("[SetOffset0xE8_Bit23:22] value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			//
			else if(section == CFG_SECTION_PSEL_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[PSEL] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_PSEL;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_PSEL);
				}
				else{
					DebugTrace (LITERAL("[PSEL]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_CARDSEL_ID)
			{
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[CARDSEL] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_CARDSEL;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_CARDSEL);
				}
				else{
					DebugTrace (LITERAL("[CARDSEL]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_AUXDET_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[AUXDET] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_AUXDET;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_AUXDET);
				}
				else{
					DebugTrace (LITERAL("[AUXDET]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_CLKREQ_FORCE_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[EnableClockRequest] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(value == 0){
					pconfigSetting->chipSetting &= ~MASK_BIT_CLKREQ_FORCE;
					pconfigSetting->chipSetting |= MASK_BIT_CLKREQ_FORCE;
				}
				else if(value == 1){
					pconfigSetting->chipSetting &= ~MASK_BIT_CLKREQ_FORCE;
				}
				else{
					DebugTrace (LITERAL("[EnableClockRequest]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_SERIALNUM_CAP_EN_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[SerialNumCapEnable] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_SERIALNUM_CAP_EN;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_SERIALNUM_CAP_EN);
				}
				else{
					DebugTrace (LITERAL("[SerialNumCapEnable]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_PCI_TXSWING_CTRL_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[PCIeTxSwingControl] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_PCI_TXSWING_CTRL;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_PCI_TXSWING_CTRL);
				}
				else{
					DebugTrace (LITERAL("[PCIeTxSwingControl]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_OSC_THROUGH_MODE_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[EnableOSCThroughMode] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if((value == 0) || (value == 1)){
					pconfigSetting->chipSetting &= ~MASK_BIT_OSC_THROUGH_MODE;
					pconfigSetting->chipSetting |= (value << SHIFT_BIT_OSC_THROUGH_MODE);
				}
				else{
					DebugTrace (LITERAL("[EnableOSCThroughMode]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else if(section == CFG_SECTION_FWUPDATE_ID){
				if(Str2Val(line, BASE_UCHAR, &value, 16) != 0){
					DebugTrace (LITERAL("[FWUpdateProperty] is unknown format! (line = %d)\n"),line_count);
					break;
				}

				if(value == 1){
					*fwupdate = TRUE;
				}
				else if(value == 0){
					*fwupdate = FALSE;
				}
				else{
					DebugTrace (LITERAL("[FWUpdateProperty]value is Outside set range! (value = %d)\n"), value);
					break;
				}
				section = CFG_SECTION_NON_ID;
			}
			else{
				break;
			}
		}

	line_count++;
	}

	FCLOSE(fp);

	// Error
	if(!fEof){
		return XHCSTS_CFGFILE_ERR;
	}

	if((!bSSID) || (!bSSVID)){
		DebugTrace(LITERAL("Please specify SubSystemID & SubSytemVenderID to config file.\n"));
		return XHCSTS_CFGFILE_ERR;
	}

	return XHCSTS_SUCCESS;
}

