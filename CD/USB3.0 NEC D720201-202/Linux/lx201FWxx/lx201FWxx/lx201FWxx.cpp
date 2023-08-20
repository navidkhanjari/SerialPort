/*
	File Name : lx201FWxx.cpp
	
	uPD720201 & uPD720202 SerialROM Writer

	* Copyright (C) 2011 Renesas Electronics Corporation

*/
///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2011-05-20	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////

#include "define.h"
#include "log.h"

//********************************************************************************//
//	Global
//********************************************************************************//

// Application Name
TCHAR g_AppName[MAX_FNAME];

// Command line info
COMMAND_INFO g_Command;
TCHAR g_CommandLine[MAX_PATH * 2];

// Selected Device
int g_SelDev = -1;
unsigned short g_DevAddr = 0xFFFF;

// Enum Device Info
PXHC_PCIDEV_INFO g_DevInfo = NULL;
int g_DevInfoOffset = 0;

int g_FindNum = 0;

int g_findDevFlag = DEVICE_ID_TYPE_D201;
//
// SROM Info
//
enum{
	SROM_AUTO=0,
	SROM_AT25F512B,
	SROM_25Lxx,
	SROM_PM25LD,
	SROM_W25XxxV,
	SROM_A25Lxxx,
	SROM_EN25Fxx,
	SROM_25Lxx21E,
	SROM_M25P10A,
	SROM_M25P11A,
	SROM_M25LV512A,
	SROM_M25LV010A,
	SROM_M25P12,
	SROM_M25P13,
	SROM_M25LV020A,
	SROM_M25LV040A,
	SROM_SST25VF048A,
	SROM_SST25VF049A,
	SROM_SST25VF043A,
	SROM_SST25VFxxxB,
	SROM_END,
	SROM_25LC512,
	SROM_USER,
	SROM_GETINFO,
	SROM_25LC512_DBG = 255,
	SROM_END_EX
};

enum{
	SROM_TYPE0=0,
	SROM_TYPE1,
	SROM_TYPE2,
	SROM_TYPE3,
	SROM_TYPE4,
	SROM_TYPE5,
	SROM_TYPE6,
	SROM_TYPE7,
	SROM_TYPE8,
	SROM_TYPE_END
};

XHC_SROM_INFO g_SRomInfo[] = 
{
	// Size,Pagesize,devCode
	{0,0,0},			 							// SROM_AUTO
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// AT25F512B
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// 25LxxÅA25Lxxx6 Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// PM25LD Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// W25XxxV Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// A25Lxx Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// EN25Fxx Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// 25Lxx21E Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// M25Pxx-A Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// M25Pxx-A Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// PM25LV512A
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// PM25LV010A
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// M25Pxx Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// M25Pxx Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// PM25LV020A
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// PM25LV040A
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// SST25VFxxx/A Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// SST25VFxxx/A Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// SST25VFxxx/A Series
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// SST25VFxxxB Series
	{0,0,0},										// SROM_END (image gen)
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,0x0080},						// SROM_25LC512
	{MAX_SROM_SIZE,SROM_PAGE_SIZE,SROM_CODE_FROM},	// SROM_GETINFO (default param)
};

// SROM Type table

SROM_TYPE_TABLE g_SRomType[] = 
{
	// Type, Parameter,  ProductId
	{0UL, 0UL, 0UL},					// AUTO
	{1UL, 0x00000700UL, 0x001F6500UL},	// AT25F512B
	{2UL, 0x00000700UL, 0x00C22000UL},	// 25LxxÅA25Lxxx6 Series
	{2UL, 0x00000700UL, 0x019D2000UL},	// PM25LD Series
	{3UL, 0x00000700UL, 0x00EF3000UL},	// W25XxxV Series
	{3UL, 0x00000700UL, 0x00373000UL},	// A25Lxx Series
	{3UL, 0x00000700UL, 0x001C3100UL},	// EN25Fxx Series
	{4UL, 0x00000500UL, 0x00C22200UL},	// 25Lxx21E Series
	{5UL, 0x00000750UL, 0x00202010UL},	// M25Pxx-A Series
	{5UL, 0x00000750UL, 0x00202011UL},	// M25Pxx-A Series
	{5UL, 0x00000750UL, 0x009D7F7BUL},	// PM25LV512A
	{5UL, 0x00000750UL, 0x009D7F7CUL},	// PM25LV010A
	{6UL, 0x00000760UL, 0x00202012UL},	// M25Pxx Series
	{6UL, 0x00000760UL, 0x00202013UL},	// M25Pxx Series
	{6UL, 0x00000760UL, 0x009D7F7DUL},	// PM25LV020A
	{6UL, 0x00000760UL, 0x009D7F7EUL},	// PM25LV040A
	{7UL, 0x00010791UL, 0x00BF0048UL},	// SST25VFxxx/A Series
	{7UL, 0x00010791UL, 0x00BF0049UL},	// SST25VFxxx/A Series
	{7UL, 0x00010791UL, 0x00BF0043UL},	// SST25VFxxx/A Series
	{8UL, 0x00010792UL, 0x00BF2500UL},	// SST25VFxxxB Series
};

LPCTSTR g_SRomName[SROM_END] =
{
	LITERAL(" Unknown SROM, Product ID = 0x%08lX\n"),
	LITERAL(" AT25F512B(ATMEL), Product ID = 0x001F6500\n"),
	LITERAL(" 25Lxx/6 series(Macronix), Product ID = 0x%08lX\n"),
	LITERAL(" Pm25LD512C/512C2(Chingis), Product ID = 0x%08lX\n"),
	LITERAL(" W25XxxV series(Winbond), Product ID = 0x%08lX\n"),
	LITERAL(" A25Lxxx series(AMIC), Product ID = 0x%08lX\n"),
	LITERAL(" EN25Fxx Series(EON), Product ID = 0x%08lX\n"),
	LITERAL(" 25Lxx21E series(Macronix), Product ID = 0x%08lX\n"),
	LITERAL(" M25Pxx-A series(Numonyx), Product ID = 0x00202010\n"),
	LITERAL(" M25Pxx-A series(Numonyx), Product ID = 0x00202011\n"),
	LITERAL(" PM25LV512A(Chingis), Product ID = 0x009D7F7B\n"),
	LITERAL(" PM25LV010A(Chingis), Product ID = 0x009D7F7C\n"),
	LITERAL(" M25Pxx series(Numonyx), Product ID = 0x00202012\n"),
	LITERAL(" M25Pxx series(Numonyx), Product ID = 0x00202013\n"),
	LITERAL(" PM25LV020A(Chingis), Product ID = 0x009D7F7D\n"),
	LITERAL(" PM25LV040A(Chingis), Product ID = 0x009D7F7E\n"),
	LITERAL(" SST25VFxxx/A series(SST), Product ID = 0x00BF0048\n"),
	LITERAL(" SST25VFxxx/A series(SST), Product ID = 0x00BF0049\n"),
	LITERAL(" SST25VFxxx/A series(SST), Product ID = 0x00BF0043\n"),
	LITERAL(" SST25VFxxxB series(SST),  Product ID = 0x%08lX\n"),
};

unsigned long g_setSromParam;

//
// Status Convert Table
//
STATUS_CONV_ENTRY g_StatusConvTable[] = 
{
	// XHC Status				Application status	
	{XHCSTS_SUCCESS,			W201STS_SUCCESS			},
	{XHCSTS_ERROR,				W201STS_ERROR			},
	{XHCSTS_FIND_DEVICE_ERROR,	W201STS_DEVICE_FOUND_ERR},
	{XHCSTS_NOT_OPENED,			W201STS_ERROR			},
	{XHCSTS_ALREADY_OPEN,		W201STS_ERROR			},
	{XHCSTS_BUSY,				W201STS_ERROR			},
	{XHCSTS_PARAMETER_ERROR,	W201STS_ERROR			},
	{XHCSTS_MEMORY_ERROR,		W201STS_MEMORY_ERR		},
	{XHCSTS_INVALID_REQUEST,	W201STS_DEVACCESS_ERR	},
	{XHCSTS_LOADUFW_ERROR,		W201STS_DEVACCESS_ERR	},
	{XHCSTS_DEVRESET_ERROR,		W201STS_DEVACCESS_ERR	},
	{XHCSTS_NOT_INITIALIZE,		W201STS_ERROR			},
	{XHCSTS_DRVLOAD_ERROR,		W201STS_ERROR			},
	{XHCSTS_DRVUNLOAD_ERROR,	W201STS_ERROR			},
	{XHCSTS_DEVMGR_ERROR,		W201STS_ERROR			},
	{XHCSTS_MEMFILE_NOT_FOUND,	W201STS_FILE_NOT_FOUND	},
	{XHCSTS_MEMFILE_ERR,		W201STS_IMGFILE_ERR		},
	{XHCSTS_CFGFILE_NOT_FOUND,	W201STS_FILE_NOT_FOUND	},
	{XHCSTS_CFGFILE_ERR,		W201STS_CONFIGFILE_ERR	},
	{XHCSTS_EUIFILE_NOT_FOUND,	W201STS_FILE_NOT_FOUND	},
	{XHCSTS_EUIFILE_ERR,		W201STS_EUI64FILE_ERR	},
	{XHCSTS_SROM_HEAD_ERROR,	W201STS_SROMFORMAT_ERR	},
	{XHCSTS_SROM_FORMAT_ERROR,	W201STS_SROMFORMAT_ERR	},
	{XHCSTS_SROM_SIZE_OVER,		W201STS_SROMFORMAT_ERR	},
	{XHCSTS_UNKNOWN_SROM,		W201STS_UNKNOWN_SROM	}
};

//********************************************************************************//
//	function
//********************************************************************************//
int GetFwVersion(LPCTSTR lpMemPath, unsigned char* fwRev);

///////////////////////////////////////////////////////////////////////////////
//		Common Function
///////////////////////////////////////////////////////////////////////////////
//
int ConvertStatus(int xhcsts)
{
	int i,count;

	count = sizeof(g_StatusConvTable) / sizeof(g_StatusConvTable[0]);

	for(i=0;i<count;i++){

		if(xhcsts == g_StatusConvTable[i].libsts){
			return g_StatusConvTable[i].appsts;
		}
	}

	return W201STS_ERROR;
}

//
// Check command line
//
int CheckCommandLine(int argc, TCHAR *argv[])
{
	unsigned long preCmd;
	unsigned int index;
	int ok;
	TCHAR *p, *tok, *endstr;
	TCHAR tmp[STR_ADDRESS_LEN];
	unsigned long num;
	size_t flength;

	// Init Command Info
	memset(&g_Command,0,sizeof(COMMAND_INFO));
	g_Command.devAddr = 0xFFFF;
	g_Command.srom = SROM_END;

	preCmd = 0;
	index  = 0;
	ok = 1;

	while(--argc){
		p = *++argv;
		// Command
		if(p[0] == LITERAL('/')){

			if(!ok){
				PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
				return W201STS_CMDLINE_ERR;
			}
			
			ok = 0;
			preCmd = 0;
			index = 0;
			
			// write command
			if(STRICMP(p, LITERAL("/write")) == 0){
				// except erase
				if(g_Command.cmd & W201CMD_ERSMASK1){
					PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
					return W201STS_CMDLINE_ERR;
				}
				
				g_Command.cmd |= W201CMD_WRITE;
				preCmd = W201CMD_WRITE;
			}
			// write command
			else if(STRICMP(p, LITERAL("/writeall")) == 0){
				// except erase
				if(g_Command.cmd & W201CMD_ERSMASK1){
					PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
					return W201STS_CMDLINE_ERR;
				}
				
				g_Command.cmd |= W201CMD_WRITEALL;
				preCmd = W201CMD_WRITEALL;
			}
			// log command
			else if(STRICMP(p, LITERAL("/log")) == 0){
				g_Command.cmd |= W201CMD_LOG;
				preCmd = W201CMD_LOG;
				
				STRCPY(g_Command.fname4, MAX_PATH, DEF_LOG_NAME);	// default copy
				
				ok = 1;
			}
			// address
			else if(STRICMP(p, LITERAL("/address")) == 0){
				if((g_Command.cmd & W201CMD_MASK) & ~(W201CMD_ADDRMASK)){
					PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
					return W201STS_CMDLINE_ERR;
				}
				g_Command.cmd |= W201CMD_ADDRESS;
				preCmd = W201CMD_ADDRESS;
			}
			// erase command
			else if(STRICMP(p, LITERAL("/erase")) == 0){
				// except write or fw or config or eui64 or imagewrite
				if((g_Command.cmd & W201CMD_MASK) & ~(W201CMD_ERSMASK2)){
					PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
					return W201STS_CMDLINE_ERR;
				}

				g_Command.cmd |= W201CMD_ERASE;
				ok = 1;
			}
			// noerase command
			else if(STRICMP(p, LITERAL("/noerase")) == 0){
				// except write or fw or config or eui64 or imagewrite
				if((g_Command.cmd & W201CMD_MASK) & ~(W201CMD_NOERSMASK)){
					PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
					return W201STS_CMDLINE_ERR;
				}

				g_Command.cmd |= W201CMD_NOERASE;
				ok = 1;
			}

			// dump commnad
			else if(STRICMP(p, LITERAL("/dump")) == 0){
				if(g_Command.cmd & W201CMD_MASK){
					PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
					return W201STS_CMDLINE_ERR;
				}

				g_Command.cmd |= W201CMD_DUMP;
				preCmd = W201CMD_DUMP;
			}
			// verify commnad
			else if(STRICMP(p, LITERAL("/verify")) == 0){
				if(g_Command.cmd & W201CMD_MASK){
					PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
					return W201STS_CMDLINE_ERR;
				}

				g_Command.cmd |= W201CMD_VERIFY;
				preCmd = W201CMD_VERIFY;
			}
			// srom command
			else if(STRICMP(p, LITERAL("/srom")) == 0){
				if(g_Command.cmd & W201CMD_ROMINFO){
					PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
					return W201STS_CMDLINE_ERR;
				}

				g_Command.cmd |= W201CMD_ROMTYPE;
				preCmd = W201CMD_ROMTYPE;
			}
			// usage commnad
			else if(STRICMP(p, LITERAL("/?")) == 0){
				if((argc == 1) || (g_Command.cmd == 0)){
					g_Command.cmd |= W201CMD_USAGE;
					ok = 1;
				}
				else{
					PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
					return W201STS_CMDLINE_ERR;
				}
			}
			// unknown
			else{
				PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
				return W201STS_CMDLINE_ERR;
			}
		}
		else if(preCmd){
			switch(preCmd){
				case W201CMD_WRITE:
				case W201CMD_VERIFY:
				case W201CMD_WRITEALL:
					if(index >= 3){
						PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
						return W201STS_CMDLINE_ERR;
					}
					else{
						if(STRLEN(p) >= MAX_PATH){
							PRINTF(LITERAL("Parameter too long = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}

						flength = STRLEN(p);
						if(index  == 0){
							STRCPY(g_Command.fname1, MAX_PATH,p);
							if(STRICMP(&p[flength-4], LITERAL(".mem")) != 0){
								PRINTF(LITERAL("An extension is Invalid : \"%s\"\n"),p);
								return W201STS_FWFILE_ERR;
							}
							ok = 1;
						}
						else if(index == 1){
							STRCPY(g_Command.fname2, MAX_PATH,p);
							if(STRICMP(&p[flength-4], LITERAL(".ini")) != 0){
								PRINTF(LITERAL("An extension is Invalid : \"%s\"\n"),p);
								return W201STS_CONFIGFILE_ERR;
							}
						}
						else if(index == 2){
							STRCPY(g_Command.fname3, MAX_PATH,p);
							if(STRICMP(&p[flength-4], LITERAL(".eui")) != 0){
								PRINTF(LITERAL("An extension is Invalid : \"%s\"\n"),p);
								return W201STS_EUI64FILE_ERR;
							}
						}
					}
					break;
				case W201CMD_LOG:
					if(index == 0){
	
						if(STRLEN(p) >= MAX_PATH){
							PRINTF(LITERAL("Parameter too long = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}

						STRCPY(g_Command.fname4, MAX_PATH,p);

					}
					else{
						PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
						return W201STS_CMDLINE_ERR;
					}
					break;

				case W201CMD_ADDRESS:
					if(index == 0){
						
						if(STRLEN(p) > 8){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
						
						STRCPY(tmp, STR_ADDRESS_LEN, p);
						
						// BusID
						tok = STRTOK(tmp, LITERAL("-") , &context);

						if(!tok){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
						
						num = STRTOUL(tok,&endstr,16);
						
						if((endstr == tok) || (endstr != &tok[STRLEN(tok)])){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
						else if(num >= 0x100){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
						
						g_Command.devAddr = (USHORT)((num & 0x00FF) << 8);

						// DeviceID
						tok = STRTOK(NULL, LITERAL("-"), &context);

						if(!tok){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}

						num = STRTOUL(tok,&endstr,16);

						if((endstr == tok) || (endstr != &tok[STRLEN(tok)])){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
						else if(num >= 32){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}

						g_Command.devAddr |= (USHORT)((num & 0x001F) << 3);

						// Function
						tok = STRTOK(NULL, LITERAL("\0"), &context);

						if(!tok){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}

						num = STRTOUL(tok,&endstr,16);

						if((endstr == tok) || (endstr != &tok[STRLEN(tok)])){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
						else if(num >= 8){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
						g_Command.devAddr |= (USHORT)(num & 0x0007);
						ok = 1;

					}
					else{
						PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
						return W201STS_CMDLINE_ERR;
					}
					break;

				case W201CMD_DUMP:
					if(index == 0){
						if(STRLEN(p) >= MAX_PATH){
							PRINTF(LITERAL("Parameter too long = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}

						STRCPY(g_Command.fname5, MAX_PATH,p);
						ok = 1;
					}
					else{
						PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
						return W201STS_CMDLINE_ERR;
					}
					break;
				case W201CMD_ROMTYPE:
					if(index == 0){
						
						if(STRICMP(p, LITERAL("?")) == 0){
							g_Command.srom = SROM_GETINFO;
							ok = 1;
						}
						else{
							num = 0;
							num = STRTOUL(p,&endstr,0);
							
							if((endstr == p) || (endstr != &p[STRLEN(p)])){
								PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
								return W201STS_CMDLINE_ERR;
							}
							else if((num >= SROM_AUTO) && (num < SROM_END)){
								g_Command.srom = (UINT)num;
								ok = 1;
							}
							// for Debug
							else if(num == SROM_25LC512_DBG){
								g_Command.srom = (UINT)SROM_25LC512;
								ok = 1;
							}
							else{
								PRINTF(LITERAL("Unknown Serial ROM Type = \"%s\"\n"),p);
								return W201STS_CMDLINE_ERR;
							}
						}
					}
					else{
						PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
						return W201STS_CMDLINE_ERR;
					}
					break;

				case W201CMD_ROMINFO:
					// Memoery Size
					if(index == 0){
						num = 0;
						num = STRTOUL(p,&endstr,0);
						
						if((endstr == p) || (endstr != &p[STRLEN(p)])){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
						else if(num == 0){
							g_SRomInfo[SROM_USER].Size = MAX_SROM_SIZE;
						}
						else if((num > 0) && (num <= MAX_SROM_SIZE)){
							g_SRomInfo[SROM_USER].Size = (ULONG)num;
						}
						else{
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
					}
					// Page Size
					else if(index == 1){
						num = 0;
						num = STRTOUL(p,&endstr,0);
						
						if((endstr == p) || (endstr != &p[STRLEN(p)])){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
						else if(num == 0){
							g_SRomInfo[SROM_USER].Page = SROM_PAGE_SIZE;
						}
						else if((num <= USHRT_MAX) && (num <= g_SRomInfo[SROM_USER].Size)){
							g_SRomInfo[SROM_USER].Page = (USHORT)num;
						}
						else{
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
					}
					// Device Code
					else if(index == 2){
						num = 0;
						num = STRTOUL(p,&endstr,0);
						
						if((endstr == p) || (endstr != &p[STRLEN(p)])){
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
						else if(num <= USHRT_MAX){
							g_SRomInfo[SROM_USER].Code = (USHORT)num;
							g_Command.srom = SROM_USER;
							ok = 1;
						}
						else{
							PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
							return W201STS_CMDLINE_ERR;
						}
					}
					else{
						PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
						return W201STS_CMDLINE_ERR;
					}
					break;
				}
			index++;
		}
		// unknown
		else{
			PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
			return W201STS_CMDLINE_ERR;
		}
	}

	if(!ok){
		PRINTF(LITERAL("Parameter error = \"%s\"\n"),p);
		return W201STS_CMDLINE_ERR;
	}

	if((g_Command.cmd & W201CMD_EXMASK2) == 0){
		g_Command.cmd |= W201CMD_ENUM;
	}
	else if((g_Command.cmd & W201CMD_EXMASK) == 0){
		PRINTF(LITERAL("Command action not found!\n"));
		return W201STS_CMDLINE_ERR;
	}
	else if((g_Command.cmd & W201CMD_NOERASE) && 
			((g_Command.cmd & W201CMD_NOERSMASK) == 0)){
		PRINTF(LITERAL("Parameter error = \"/noerase\"\n"));
		return W201STS_CMDLINE_ERR;
	}
	else if(g_Command.srom == SROM_GETINFO){
		PRINTF(LITERAL("Parameter error = \"/srom ?\"\n"));
		return W201STS_CMDLINE_ERR;
	}
	
	// Use Serial ROM
	if(g_Command.cmd & W201CMD_SROMMASK){
		
		if(((g_Command.cmd & W201CMD_ROMTYPE) == 0) &&
		   ((g_Command.cmd & W201CMD_ROMINFO) == 0)){
			PRINTF(LITERAL("Please specify \"/srom\" option.\n"));
			return W201STS_CMDLINE_ERR;
		}
	}

	// Use address
	if(g_Command.cmd & W201CMD_ADDRESS){
		if((g_Command.cmd & W201CMD_MASK) & ~(W201CMD_ADDRMASK)){
			PRINTF(LITERAL("Parameter error = \"/address\"\n"));
			return W201STS_CMDLINE_ERR;
		}
	}

#ifdef _DEBUG
	DebugTrace(LITERAL("Command Line Parameter\n"));
	DebugTrace(LITERAL("Command = 0x%08lX\n"),g_Command.cmd);
	if(g_Command.fname1[0]){
		DebugTrace(LITERAL("fname1 = \"%s\"\n"),g_Command.fname1);
	}
	else{
		DebugTrace(LITERAL("fname1 = NULL\n"));
	}
	if(g_Command.fname2[0]){
		DebugTrace(LITERAL("fname2 = \"%s\"\n"),g_Command.fname2);
	}
	else{
		DebugTrace(LITERAL("fname2 = NULL\n"));
	}
	if(g_Command.fname3[0]){
		DebugTrace(LITERAL("fname3 = \"%s\"\n"),g_Command.fname3);
	}
	else{
		DebugTrace(LITERAL("fname3 = NULL\n"));
	}	
	if(g_Command.fname4[0]){
		DebugTrace(LITERAL("fname4 = \"%s\"\n"),g_Command.fname4);
	}
	else{
		DebugTrace(LITERAL("fname4 = NULL\n"));
	}
	if(g_Command.fname5[0]){
		DebugTrace(LITERAL("fname5 = \"%s\"\n"),g_Command.fname5);
	}
	else{
		DebugTrace(LITERAL("fname5 = NULL\n"));
	}
	DebugTrace(LITERAL("devAddr = 0x%04X\n"),g_Command.devAddr);
	DebugTrace(LITERAL("romtype = %d\n"),g_Command.srom);

#endif

	return W201STS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//		Device Access Function
///////////////////////////////////////////////////////////////////////////////
//
// Device Find
//
int FindDevice(void)
{
	int xhcsts;
	unsigned int nFind;
	int num = 0;
	int i;
	int FindNumTmp = 0;
	PXHC_PCIDEV_INFO DevInfo_tmp = NULL;
	PXHC_PCIDEV_INFO GetDevInfo  = NULL;

	if(g_DevInfo){
		if(g_FindNum != 0){
			/* information on 201 isn't overwritten in FindDevice of 202 */
			DevInfo_tmp = (PXHC_PCIDEV_INFO)malloc(sizeof(XHC_PCIDEV_INFO) * g_FindNum);
			FindNumTmp  = g_FindNum;
			for(i=0;i<g_FindNum;i++){
				DevInfo_tmp[i].DevAddr           = g_DevInfo[i].DevAddr;
				DevInfo_tmp[i].SubSystemVendorID = g_DevInfo[i].SubSystemVendorID;
				DevInfo_tmp[i].SubSystemID       = g_DevInfo[i].SubSystemID;
				DevInfo_tmp[i].RevisionID        = g_DevInfo[i].RevisionID;
			}
		}
	}

	g_FindNum = 0;
	do{
		if(g_findDevFlag == DEVICE_ID_TYPE_D201){
			xhcsts = xHcFindDevice(
							W201_VENDOR_ID,
							W201_DEVICE_ID,
							GetDevInfo,
							num,
							&nFind);
		}
		else{
			xhcsts = xHcFindDevice(
							W202_VENDOR_ID,
							W202_DEVICE_ID,
							GetDevInfo,
							num,
							&nFind);
		}
		if(xhcsts != XHCSTS_SUCCESS){
			PRINTF(LITERAL("Device Enumration Error! (%d)\n"), xhcsts);
			return W201STS_DEVICE_FOUND_ERR;
		}

		DebugTrace(LITERAL("%d devices found. (buffer = %d devices)\n"),nFind, num);

		if((nFind) && (num != (signed int)nFind)){
			if(GetDevInfo){
				free(GetDevInfo);
				GetDevInfo = NULL;
			}

			GetDevInfo = (PXHC_PCIDEV_INFO)malloc(sizeof(XHC_PCIDEV_INFO) * nFind);

			if(!GetDevInfo){
				PRINTF(LITERAL("Memory allocate error = %d\n"), (sizeof(XHC_PCIDEV_INFO) * nFind));
				return W201STS_MEMORY_ERR;
			}

			num = (int)nFind;
		}
		else{
			// success 
			g_FindNum = nFind;

			if(g_DevInfo){
				free(g_DevInfo);
				g_DevInfo = NULL;
			}

			g_DevInfo = (PXHC_PCIDEV_INFO)malloc(sizeof(XHC_PCIDEV_INFO) * (FindNumTmp + nFind));
			if(FindNumTmp != 0){
				/* Information on 201 is copied. */
				for(i=0;i<FindNumTmp;i++){
					g_DevInfo[i].DevAddr           = DevInfo_tmp[i].DevAddr;
					g_DevInfo[i].SubSystemVendorID = DevInfo_tmp[i].SubSystemVendorID;
					g_DevInfo[i].SubSystemID       = DevInfo_tmp[i].SubSystemID;
					g_DevInfo[i].RevisionID        = DevInfo_tmp[i].RevisionID;
				}
			}

			/* Information on 202 is copied. */
			for(i=0;i<(int)nFind;i++){
				g_DevInfo[i+FindNumTmp].DevAddr           = GetDevInfo[i].DevAddr;
				g_DevInfo[i+FindNumTmp].SubSystemVendorID = GetDevInfo[i].SubSystemVendorID;
				g_DevInfo[i+FindNumTmp].SubSystemID       = GetDevInfo[i].SubSystemID;
				g_DevInfo[i+FindNumTmp].RevisionID        = GetDevInfo[i].RevisionID;
			}

			if(DevInfo_tmp){
				free(DevInfo_tmp);
				DevInfo_tmp = NULL;
			}
			if(GetDevInfo){
				free(GetDevInfo);
				GetDevInfo = NULL;
			}

			break;
		}
	}while(1);

	return W201STS_SUCCESS;

}

//
// Get Serial ROM Info
//
int GetSromInfo(unsigned short devAddr, BOOL bCheck)
{
	XHC_SROM_INFO sromInfo;
	int i;
	int xhcsts;
	int ret = W201STS_SUCCESS;
	unsigned long codeMask;
	unsigned long productId;
	unsigned long setParam = PRODUCT_ERROR_PARAM;
	int loop;

	xhcsts = xHcDetectSerialROM(devAddr,
								&sromInfo);
	codeMask = (sromInfo.Code & PRODUCT_NAME_xx_MASK);
	productId = sromInfo.Code;
	if((codeMask == g_SRomType[SROM_25Lxx].ProductId) || (codeMask == g_SRomType[SROM_PM25LD].ProductId)  ||
	(codeMask == g_SRomType[SROM_W25XxxV].ProductId)  || (codeMask == g_SRomType[SROM_A25Lxxx].ProductId) ||
	(codeMask == g_SRomType[SROM_25Lxx21E].ProductId) || (codeMask == g_SRomType[SROM_SST25VFxxxB].ProductId) ||
	(codeMask == g_SRomType[SROM_EN25Fxx].ProductId)){
		sromInfo.Code = codeMask;
	}

	switch((ULONG)g_Command.srom){
		case SROM_TYPE0:
		case SROM_GETINFO:
			for(loop = SROM_AT25F512B; loop < SROM_END; loop++){
				if(sromInfo.Code == g_SRomType[loop].ProductId){
					setParam = g_SRomType[loop].Parameter;
					g_SRomInfo[SROM_TYPE0].Size = g_SRomInfo[loop].Size;
					g_SRomInfo[SROM_TYPE0].Page = g_SRomInfo[loop].Page;
					break;
				}
			}
			break;
		case SROM_TYPE1:
			if(sromInfo.Code == g_SRomType[SROM_AT25F512B].ProductId){
				setParam = g_SRomType[SROM_AT25F512B].Parameter;
			}
			break;
		case SROM_TYPE2:
			/* A definition of ProductID masks because "00C2_20XXh" and for 8Bit are the unsettled value */
			if((sromInfo.Code == g_SRomType[SROM_25Lxx].ProductId) ||
			   (sromInfo.Code == g_SRomType[SROM_PM25LD].ProductId)){
				setParam = g_SRomType[SROM_25Lxx].Parameter;
			}
			break;
		case SROM_TYPE3:
			if((sromInfo.Code == g_SRomType[SROM_W25XxxV].ProductId) ||
			   (sromInfo.Code == g_SRomType[SROM_A25Lxxx].ProductId) ||
			   (sromInfo.Code == g_SRomType[SROM_EN25Fxx].ProductId)){
				setParam = g_SRomType[SROM_W25XxxV].Parameter;
			}
			break;
		case SROM_TYPE4:
			if(sromInfo.Code == g_SRomType[SROM_25Lxx21E].ProductId){
				setParam = g_SRomType[SROM_25Lxx21E].Parameter;
			}
			break;
		case SROM_TYPE5:
			if((sromInfo.Code == g_SRomType[SROM_M25P10A].ProductId)   ||
			   (sromInfo.Code == g_SRomType[SROM_M25P11A].ProductId)   ||
			   (sromInfo.Code == g_SRomType[SROM_M25LV512A].ProductId) ||
			   (sromInfo.Code == g_SRomType[SROM_M25LV010A].ProductId)){
				setParam = g_SRomType[SROM_M25P10A].Parameter;
			}
			break;
		case SROM_TYPE6:
			if((sromInfo.Code == g_SRomType[SROM_M25P12].ProductId)    ||
			   (sromInfo.Code == g_SRomType[SROM_M25P13].ProductId)    ||
			   (sromInfo.Code == g_SRomType[SROM_M25LV020A].ProductId) ||
			   (sromInfo.Code == g_SRomType[SROM_M25LV040A].ProductId)){
				setParam = g_SRomType[SROM_M25P12].Parameter;
			}
			break;
		case SROM_TYPE7:
			if((sromInfo.Code == g_SRomType[SROM_SST25VF048A].ProductId) ||
			   (sromInfo.Code == g_SRomType[SROM_SST25VF049A].ProductId) ||
			   (sromInfo.Code == g_SRomType[SROM_SST25VF043A].ProductId)){
				setParam = g_SRomType[SROM_SST25VF048A].Parameter;
			}
			break;
		case SROM_TYPE8:
			if(sromInfo.Code == g_SRomType[SROM_SST25VFxxxB].ProductId){
				setParam = g_SRomType[SROM_SST25VFxxxB].Parameter;
			}
			break;
		default:
			setParam = PRODUCT_ERROR_PARAM;
			break;
	}

	if(setParam != PRODUCT_ERROR_PARAM){
		sromInfo.Code = setParam;
		xhcsts = xHcSetSerialROMInfo(devAddr,
								&sromInfo);

		if(xhcsts == XHCSTS_SUCCESS){
			g_setSromParam = setParam;
			DebugTrace(LITERAL("     SROM::Type = %04lX\n"),setParam);
		}
		else{
			PRINTF(LITERAL("Can't set Serial ROM Information!\n"));
		}
	}
	else{
		xhcsts = XHCSTS_UNKNOWN_SROM;
	}

	if(xhcsts == XHCSTS_SUCCESS){
		i = SROM_AUTO + 1;

		if(bCheck){
			if(codeMask == g_SRomType[SROM_25Lxx].ProductId){
				// Defined SROM
				PRINTF(g_SRomName[SROM_25Lxx], productId);
			}
			else if(codeMask == g_SRomType[SROM_PM25LD].ProductId){
				PRINTF(g_SRomName[SROM_PM25LD], productId);
			}
			else if(codeMask == g_SRomType[SROM_W25XxxV].ProductId){
				PRINTF(g_SRomName[SROM_W25XxxV], productId);
			}
			else if(codeMask == g_SRomType[SROM_A25Lxxx].ProductId){
				PRINTF(g_SRomName[SROM_A25Lxxx], productId);
			}
			else if(codeMask == g_SRomType[SROM_EN25Fxx].ProductId){
				PRINTF(g_SRomName[SROM_EN25Fxx], productId);
			}
			else if(codeMask == g_SRomType[SROM_25Lxx21E].ProductId){
				PRINTF(g_SRomName[SROM_25Lxx21E], productId);
			}
			else if(codeMask == g_SRomType[SROM_SST25VFxxxB].ProductId){
				PRINTF(g_SRomName[SROM_SST25VFxxxB], productId);
			}
			else{
				while(i<SROM_END){
					if(g_SRomType[i].ProductId == productId){
						// Defined SROM
						PRINTF(g_SRomName[i]);
						break;
					}
					i++;
				}
			}
			// Unknown SROM
			if(i>=SROM_END){
				PRINTF(g_SRomName[0],productId);
			}
		}
	}
	else if(xhcsts == XHCSTS_UNKNOWN_SROM){
		PRINTF(LITERAL("Unsupported Serial ROM!\n"));
		ret = W201STS_UNKNOWN_SROM;
	}
	else{
		PRINTF(LITERAL("Can't get Serial ROM Information (%d)\n"), xhcsts);
		ret = W201STS_ERROR;
	}

	return ret;
}

//
// Close Device
//
int DeviceClose(unsigned short devAddr)
{
	int xhcsts;
	int ret = W201STS_SUCCESS;

	xhcsts = xHcCloseDevice(devAddr);

	if(xhcsts != XHCSTS_SUCCESS){
		ret = W201STS_ERROR;
	}

	g_SelDev = -1;

	return ret;;
}

//
// OpenDevice
//
int DeviceOpen(unsigned char fwRev)
{
	int i;
	int xhcsts;
	int ret;
	int loop;
	int findAll = 0;

	if((g_SelDev >= 0) && (g_SelDev < g_FindNum)){
		if(((g_Command.cmd & W201CMD_ADDRESS) == 0) ||
			 (g_DevInfo[g_SelDev].DevAddr == g_Command.devAddr)){
			// Already opened
			DebugTrace(LITERAL("Device was already opened!\n"));
			return W201STS_SUCCESS;
		}
		else{
			DebugTrace(LITERAL("Other device was opened!\n"));
			return W201STS_ERROR;
		}
	}

	g_SelDev = -1;
	g_DevInfoOffset = 0;

	for(loop = 0; loop < DEVICE_ID_TYPE_ALL; loop++){
		g_findDevFlag = loop;
		ret = FindDevice();

		if(ret != W201STS_SUCCESS){
			return ret;
		}

		if(g_FindNum == 0){
			if(loop > DEVICE_ID_TYPE_D201){
				if(findAll == 0){
					PRINTF(LITERAL("Device not found!\n"));
					return W201STS_DEVICE_FOUND_ERR;
				}
				else{
					if((g_Command.cmd & W201CMD_ADDRESS) != 0){
						PRINTF(LITERAL("Specified device by \"/address\" option isn't uPD720201 or uPD720202!\n"));
						return W201STS_DEVICE_FOUND_ERR;
					}
					else{
						g_findDevFlag = DEVICE_ID_TYPE_D201;
					}
				}
			}
		}
		else{
			findAll += g_FindNum;

			if((g_Command.cmd & W201CMD_ADDRESS) == 0){	/* no addressing commands */
				if(findAll != 1){
					PRINTF(LITERAL("%d device was found! Please select device by \"/address\" option.\n"), findAll);
					g_SelDev = -1;
					return W201STS_DEVICE_FOUND_ERR;
				}

				g_SelDev = 0;
				g_DevInfoOffset++;
			}
			else{
				for(i=0;i<(findAll-g_DevInfoOffset);i++){
					if(g_Command.devAddr == g_DevInfo[i+g_DevInfoOffset].DevAddr){
						g_SelDev = (i + g_DevInfoOffset);
						break;
					}
				}

				if((g_SelDev >= 0) && (g_SelDev < (findAll-g_DevInfoOffset))){
					break;
				}

				if(loop > DEVICE_ID_TYPE_D201){
					if(g_SelDev < 0){
						PRINTF(LITERAL("Specified device by \"/address\" option isn't uPD720201 or uPD720202!\n"));
						return W201STS_DEVICE_FOUND_ERR;
					}
				}
				g_DevInfoOffset += i;
			}
		}
	}

	if(!(g_Command.cmd & W201CMD_NOREV)){
		// Revision Check of 201_TARGET_REVISION
		//
		if((g_DevInfo[g_SelDev].RevisionID != W201_TARGET_REVISION_2) && 
		   (g_DevInfo[g_SelDev].RevisionID != W201_TARGET_REVISION_3) && 
		   (g_DevInfo[g_SelDev].RevisionID != W202_TARGET_REVISION)){
				if(g_DevInfo[g_SelDev].RevisionID > W201_TARGET_REVISION){
					PRINTF(LITERAL("Device not found!\n"));
					return W201STS_DEVICE_FOUND_ERR;
				}
				else{
					PRINTF(LITERAL("[Target Device]  Bus:0x%02X Device:0x%02X Function:0x%02X\n"),
								GetBus(g_DevInfo[g_SelDev].DevAddr),
								GetDev(g_DevInfo[g_SelDev].DevAddr),
								GetFunc(g_DevInfo[g_SelDev].DevAddr));
					PRINTF(LITERAL("This chip revision is %d.\n"),g_DevInfo[g_SelDev].RevisionID);
					PRINTF(LITERAL("This tool is for uPD720201(Revision %d & %d) and uPD720202(Revision %d).\n"),W201_TARGET_REVISION_2, W201_TARGET_REVISION_3, W202_TARGET_REVISION);
					PRINTF(LITERAL("Renesas Electronics release appropriate tool for this chip revision.\n"));
				}

				g_SelDev = -1;

				return W201STS_REVISION_ERR;
		}

		// FW Revision Check
		if(fwRev < 0x10){
			if(fwRev != g_DevInfo[g_SelDev].RevisionID){
				PRINTF(LITERAL("This firmware revision is %x.\n"), fwRev);
				PRINTF(LITERAL("This chip revision is %x.\n"), g_DevInfo[g_SelDev].RevisionID);
				PRINTF(LITERAL("Revision is mismatch.\n"));
				return W201STS_REVISION_ERR;
			}
		}
	}

	PRINTF(LITERAL("[Target Device]Bus:0x%02X Device:0x%02X Function:0x%02X\n"),
						GetBus(g_DevInfo[g_SelDev].DevAddr),
						GetDev(g_DevInfo[g_SelDev].DevAddr),
						GetFunc(g_DevInfo[g_SelDev].DevAddr));
	if(g_findDevFlag == DEVICE_ID_TYPE_D201){
		PRINTF(LITERAL("This Device is uPD720201(Revision %d).\n"),g_DevInfo[g_SelDev].RevisionID);
	}
	else{
		PRINTF(LITERAL("This Device is uPD720202(Revision %d).\n"),g_DevInfo[g_SelDev].RevisionID);
	}

	g_DevAddr = g_DevInfo[g_SelDev].DevAddr;

	// SROM Info Check
	if(g_Command.srom == SROM_AUTO){
		// device open
		xhcsts = xHcOpenDevice(
							g_DevAddr,
							XHC_OPMODE_UFW,
							NULL,
							g_DevInfo[g_SelDev].RevisionID
							);

		if(xhcsts != XHCSTS_SUCCESS){
			PRINTF(LITERAL("Can't open device! (%d)\n"), xhcsts);
			return W201STS_ERROR;
		}

		// SROM Info Check
		ret = GetSromInfo(g_DevAddr, FALSE);
		if(ret != W201STS_SUCCESS){
			DeviceClose(g_DevAddr);
			return ret;
		}
	}
	else{
		// device open
		xhcsts = xHcOpenDevice(
							g_DevAddr,
							XHC_OPMODE_UFW,
							&g_SRomInfo[g_Command.srom],
							g_DevInfo[g_SelDev].RevisionID
							);

		if(xhcsts != XHCSTS_SUCCESS){
			PRINTF(LITERAL("Can't open device! (%d)\n"), xhcsts);
			return W201STS_ERROR;
		}

		// SROM Info Check
		ret = GetSromInfo(g_DevAddr, FALSE);
		if(ret != W201STS_SUCCESS){
			DeviceClose(g_DevAddr);
			return ret;
		}
	}
	g_FindNum = findAll;

	return W201STS_SUCCESS;
}

//
// SRom Verify
//
int SRomVerify(UCHAR* pBuf, unsigned long size)
{
	unsigned long readSize;
	int xhcsts;
	int ret;
	unsigned char* pComp = NULL;

	DebugTraceFunction(LITERAL(" ++\n"));

	pComp = (UCHAR*)malloc(size);

	if(!pComp){
		PRINTF(LITERAL("Memory allocate error = %ld\n"),size);
		ret = W201STS_MEMORY_ERR;
		goto Exit_SRomVerify;
	}

	xHcSetExtROMAccessEnable(g_DevInfo[g_SelDev].DevAddr, (USHORT)XHC_EXTROM_ACCESS_ENABLE);

	xhcsts = xHcReadSerialROM(g_DevInfo[g_SelDev].DevAddr,
							 0,
							 size,
							 NULL,
							 NULL,
							 XHC_NO_WAIT);

	if(xhcsts != XHCSTS_SUCCESS){
        PRINTF(LITERAL ("Serial ROM read error (%d)\n"), xhcsts);
		ret = W201STS_DEVACCESS_ERR;
		goto Exit_SRomVerify;
	}

	PRINTF(LITERAL("Verify Serial ROM....  0%% "));

	do{
		SLEEP(READ_POLL_INTERVAL);

		// SROM read data
		xhcsts = xHcGetReadData(g_DevAddr,
								size,
								pComp,
								&readSize);

		if(xhcsts < XHCSTS_SUCCESS){
			PRINTF(LITERAL("\nSerial ROM read error (%d)\n"), xhcsts);
			ret = W201STS_DEVACCESS_ERR;
			goto Exit_SRomVerify;
		}

		PRINTF(LITERAL("\rVerify Serial ROM....  %3ld%% "),	((readSize * 100) / size));

	}while(xhcsts == XHCSTS_PROC_ACTIVE);

	// compare
	if(memcmp(pBuf, pComp, size) != 0){
		PRINTF(LITERAL("\nERROR::Verify mismatch!\n"));
		ret = W201STS_VERIFY_ERR;
	}
	else{
		PRINTF(LITERAL("\nVerify complete\n"));
		ret = W201STS_SUCCESS;
	}

Exit_SRomVerify:
	xHcSetExtROMAccessEnable(g_DevInfo[g_SelDev].DevAddr, (USHORT)XHC_EXTROM_ACCESS_DISABLE);

	if(pComp){
		free(pComp);
	}

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;

}


///////////////////////////////////////////////////////////////////////////////
//		Command Function
///////////////////////////////////////////////////////////////////////////////

//
// Usage
//
void Usage(int extend, int option)
{
	DebugTraceFunction(LITERAL(" ++\n"));

	PRINTF(LITERAL("Serial ROM Utility for uPD720201(Revision %d & %d) & uPD720202(Revision %d)\n"),W201_TARGET_REVISION_2, W201_TARGET_REVISION_3, W202_TARGET_REVISION);
	PRINTF(LITERAL("Usage::\n"));

	if(!extend){
		PRINTF(LITERAL("%s [ /? | /srom romtype | /write file1 [file2] [file3] | /dump file1 | \n"),g_AppName);
		PRINTF(LITERAL("          /erase | /noerase | /log [file4] | /address deviceaddress |\n"));
		PRINTF(LITERAL("          /verify file1 [file2] [file3] | /writeall file1 [file2] [file3] ]\n"));
	}

	if(!option){
		PRINTF(LITERAL("\nExample::\n"));
		PRINTF(LITERAL("\t>%s /srom 0 /write K2xxxxxx.MEM CFG_201.INI\n"),g_AppName);
		PRINTF(LITERAL("\t>%s /srom 0 /write K2xxxxxx.MEM CFG_202.INI\n"),g_AppName);
		DebugTraceFunction(LITERAL(" --\n"));

		return;
	}
	
	PRINTF(LITERAL("\nOptions::\n"));

	PRINTF(LITERAL(" /?          :Display this help message.\n"));
	PRINTF(LITERAL(" /srom       :Specify SPI-Flash-ROM device type.\n"));
	PRINTF(LITERAL(" /write      :Write both file1(.mem) and file2(.ini) to SPI Flash ROM.\n"));
	PRINTF(LITERAL("              If need to write EUI-64, specify file3(.eui).\n"));
	PRINTF(LITERAL(" /dump       :Dump ROM Image File from SPI-Flash-ROM.\n"));
	PRINTF(LITERAL(" /erase      :Erase all of the data in SPI-Flash-ROM.\n"));
	PRINTF(LITERAL(" /noerase    :Erase process is skipped. This command is used with write\n"));
	PRINTF(LITERAL("              command if needed.\n"));
	PRINTF(LITERAL(" /log        :Generates log file named file4.\n"));
	PRINTF(LITERAL("              (Default log file named \"log.txt\")\n"));
	PRINTF(LITERAL(" /address    :Specify one target device with PCI Device Address.\n"));
	PRINTF(LITERAL(" /verify     :Verify the ROM image data (Don't write to SPI-Flash-ROM).\n"));
	PRINTF(LITERAL(" /writeall   :Write both file1(.mem) and file2(.ini) to SPI Flash ROMs \n"));
	PRINTF(LITERAL("              of multi Host controllers on same system(computer).\n"));
	PRINTF(LITERAL("              If need to write EUI-64, specify file3(.eui).\n"));

	PRINTF(LITERAL("\nWhere::\n"));

	PRINTF(LITERAL("romtype       :Serial ROM type number (0,?). \n"));
	PRINTF(LITERAL("                  0. Auto Select (check serial rom type automatically)\n"));
	PRINTF(LITERAL("                  Supported Serial Flash ROM List\n"));
	PRINTF(LITERAL("                  - 25Lxx/6 series(Macronix)\n"));
	PRINTF(LITERAL("                     (Product ID = 0x00C220XX)\n"));
	PRINTF(LITERAL("                  - 25Lxx21E series(Macronix)\n"));
	PRINTF(LITERAL("                     (Product ID = 0x00C222XX)\n"));
	PRINTF(LITERAL("                  - W25XxxV series(Winbond)\n"));
	PRINTF(LITERAL("                     (Product ID = 0x00EF30XX)\n"));
	PRINTF(LITERAL("                  - Pm25LD512C/512C2(Chingis)\n"));
	PRINTF(LITERAL("                     (Product ID = 0x019D20XX)\n"));
	PRINTF(LITERAL("                  - M25Pxx-A series(Numonyx)\n"));
	PRINTF(LITERAL("                     (Product ID = 0x00202010)\n"));
	PRINTF(LITERAL("                                   0x00202011)\n"));
	PRINTF(LITERAL("                  - M25Pxx series(Numonyx)\n"));
	PRINTF(LITERAL("                     (Product ID = 0x00202012\n"));
	PRINTF(LITERAL("                                   0x00202013)\n"));
	PRINTF(LITERAL("                  - AT25F512B (ATMEL)\n"));
	PRINTF(LITERAL("                     (Product ID = 0x001F6500)\n"));
	PRINTF(LITERAL("                  - A25Lxxx series(AMIC)\n"));
	PRINTF(LITERAL("                     (Product ID = 0x003730XX)\n"));
	PRINTF(LITERAL("                  - EN25Fxx series(EON)\n"));
	PRINTF(LITERAL("                     (Product ID = 0x001C31XX)\n"));
	PRINTF(LITERAL("                  - SST25VFxxx/A series(SST)\n"));
	PRINTF(LITERAL("                     (Product ID = 0x00BF0048\n"));
	PRINTF(LITERAL("                                   0x00BF0049)\n"));

    PRINTF(LITERAL("                  ?. Check Serial ROM Information.\n"));

	PRINTF(LITERAL("file1         :Firmware File (extension is mem) released by Renesas Electroncis.\n"));
	PRINTF(LITERAL("file2         :Configuration File (extension is ini) including system dependent\n"));
	PRINTF(LITERAL("               Parameters such as PCI Subsystem ID or PCI Subsystem Vendor ID.\n"));
	PRINTF(LITERAL("               Renesas Electronics releases a sample file. User need to modify \n"));
	PRINTF(LITERAL("               those parameters in accordance with the customer system.\n"));
	PRINTF(LITERAL("file3         :EUI-64 file(lxeui.eui) that includes EUI-64(XX-XX-XX-XX-\n"));
	PRINTF(LITERAL("               XX-XX-XX-XX). If the Product is hot-plug device such as Express\n"));
	PRINTF(LITERAL("               Card, the EUI-64 must be set. As for the EUI-64, refer to the\n"));
	PRINTF(LITERAL("               following WEB site.\n"));
	PRINTF(LITERAL("file4         :LOG File Name. In case of skipping the filename4, log.txt is \n"));
	PRINTF(LITERAL("               generated on the current directory.\n"));
	PRINTF(LITERAL("deviceaddress :Format is XX-YY-ZZ. XX is PCI Bus Number(HEX). YY is PCI Device \n"));
	PRINTF(LITERAL("               Number(HEX). ZZ is PCI Function Number(HEX). If there are \n"));
	PRINTF(LITERAL("               multiple devices on the system, user can specify one device.\n"));

	PRINTF(LITERAL("\nExample::\n"));
	PRINTF(LITERAL("\t>%s\n"),g_AppName);
	PRINTF(LITERAL("\t>%s /srom 0 /write K2xxxxxx.MEM CFG_201.INI\n"),g_AppName);
	PRINTF(LITERAL("\t>%s /srom 0 /write K2xxxxxx.MEM CFG_202.INI\n"),g_AppName);
	PRINTF(LITERAL("\t>%s /srom 0 /erase\n"),g_AppName);
	PRINTF(LITERAL("\t>%s /srom 0 /address 04-00-00 /write K2xxxxxx.MEM CFG_201.INI\n"),g_AppName);
	PRINTF(LITERAL("\t>%s /srom 0 /address 04-00-00 /write K2xxxxxx.MEM CFG_202.INI\n"),g_AppName);
	PRINTF(LITERAL("\t>%s /srom 0 /verify K2xxxxxx.MEM CFG_201.INI\n"),g_AppName);
	PRINTF(LITERAL("\t>%s /srom 0 /verify K2xxxxxx.MEM CFG_202.INI\n"),g_AppName);
	PRINTF(LITERAL("\t>%s /srom 0 /writeall K2xxxxxx.MEM CFG_201.INI\n"),g_AppName);
	PRINTF(LITERAL("\t>%s /srom 0 /writeall K2xxxxxx.MEM CFG_202.INI\n"),g_AppName);

	DebugTraceFunction(LITERAL(" --\n"));
}

// Device Find
//
int EnumDevice(void)
{
	BOOL bRev = FALSE;
	XHC_DEVICE_INFO devInfo;
	int err = 0;
	int i;
	int xhcsts;
	int ret;
	int loop;
	int findAll = 0;
	int romLoop;

	DebugTraceFunction(LITERAL(" ++\n"));
	g_DevInfoOffset = 0;

	for(loop = 0; loop < DEVICE_ID_TYPE_ALL; loop++){
		ZEROMEMORY(&devInfo,sizeof(XHC_DEVICE_INFO));
		g_findDevFlag = loop;
		ret = FindDevice();

		if(ret != W201STS_SUCCESS){
			goto Exit_EnumDevice;
		}

		if(g_FindNum == 0){
			if(loop == DEVICE_ID_TYPE_D201){
				continue;
			}
			else{
				if(findAll == FALSE){
					PRINTF(LITERAL("Device not found!\n"));
					ret = W201STS_DEVICE_FOUND_ERR;
					goto Exit_EnumDevice;
				}
				else{
					ret = W201STS_SUCCESS;
				}
			}
		}

		findAll += g_FindNum;

		for(i=0; i<(findAll - g_DevInfoOffset); i++){
			if(g_DevInfo[i+g_DevInfoOffset].RevisionID > W201_TARGET_REVISION){
				PRINTF(LITERAL("Device not found!\n"));
				return W201STS_DEVICE_FOUND_ERR;
			}

			PRINTF(LITERAL("Bus:0x%02X Device:0x%02X Function:0x%02X\n"),
							GetBus(g_DevInfo[i+g_DevInfoOffset].DevAddr),
							GetDev(g_DevInfo[i+g_DevInfoOffset].DevAddr),
							GetFunc(g_DevInfo[i+g_DevInfoOffset].DevAddr));
			if(g_findDevFlag == DEVICE_ID_TYPE_D201){
				PRINTF(LITERAL("This Device is uPD720201(Revision %d).\n"),g_DevInfo[i+g_DevInfoOffset].RevisionID);
			}
			else{
				PRINTF(LITERAL("This Device is uPD720202(Revision %d).\n"),g_DevInfo[i+g_DevInfoOffset].RevisionID);
			}

			if(g_Command.srom != SROM_END){
				if(!(g_Command.cmd & W201CMD_NOREV)){
					// Revision Check
					if((g_DevInfo[i+g_DevInfoOffset].RevisionID != W201_TARGET_REVISION_2) && 
					   (g_DevInfo[i+g_DevInfoOffset].RevisionID != W201_TARGET_REVISION_3) && 
					   (g_DevInfo[i+g_DevInfoOffset].RevisionID != W202_TARGET_REVISION)){
						bRev = TRUE;
						continue;
					}
				}

				// device open
				xhcsts = xHcOpenDevice(
									g_DevInfo[i+g_DevInfoOffset].DevAddr,
									XHC_OPMODE_UFW,
	                                &g_SRomInfo[g_Command.srom],
	                                g_DevInfo[i+g_DevInfoOffset].RevisionID
									);

				if(xhcsts != XHCSTS_SUCCESS){
					PRINTF(LITERAL("Can't open device! (%d)\n"), xhcsts);
					err++;
					continue;
				}

				// SROM Info Check
				if(g_Command.srom == SROM_GETINFO){
					if(GetSromInfo(g_DevInfo[i+g_DevInfoOffset].DevAddr, TRUE) != W201STS_SUCCESS){
						err++;
					}
					// Close Device
					xHcCloseDevice(g_DevInfo[i+g_DevInfoOffset].DevAddr);

					continue;
				}
				else{
					if(GetSromInfo(g_DevInfo[i+g_DevInfoOffset].DevAddr, FALSE) != W201STS_SUCCESS){
						err++;
						// Close Device
						xHcCloseDevice(g_DevInfo[i+g_DevInfoOffset].DevAddr);

						continue;
					}
				}

				switch((g_setSromParam & SROM_ERASE_BLOCK_MASK) >> SROM_ERASE_BLOCK_SHIFT_BIT){
					case SROM_EBSZ_4KBYTE:
						devInfo.Ebsz = SROM_ERASE_BLOCK_SIZE_4K*4;
						break;
					case SROM_EBSZ_32KBYTE:
						devInfo.Ebsz = SROM_ERASE_BLOCK_SIZE_32K;
						break;
					case SROM_EBSZ_64KBYTE:
						devInfo.Ebsz = SROM_ERASE_BLOCK_SIZE_64K;
						break;
					case SROM_EBSZ_128KBYTE:
						devInfo.Ebsz = SROM_ERASE_BLOCK_SIZE_128K;
						break;
					default:
						devInfo.Ebsz = SROM_ERASE_BLOCK_SIZE_32K;
						break;
				}

				// Get Device Info
				/* Because SROM Read is done when acquiring the FW version */
				xHcSetExtROMAccessEnable(g_DevInfo[i+g_DevInfoOffset].DevAddr, (USHORT)XHC_EXTROM_ACCESS_ENABLE);
				xhcsts = xHcGetDeviceInfo(g_DevInfo[i+g_DevInfoOffset].DevAddr, &devInfo);
				xHcSetExtROMAccessEnable(g_DevInfo[i+g_DevInfoOffset].DevAddr, (USHORT)XHC_EXTROM_ACCESS_DISABLE);
				if(xhcsts != XHCSTS_SUCCESS){

					if((xhcsts == XHCSTS_SROM_HEAD_ERROR) ||
						(xhcsts == XHCSTS_SROM_FOOTER_ERROR) ||
						(xhcsts == XHCSTS_SROM_FORMAT_ERROR))
					{
						PRINTF(LITERAL("     VCD CRC Error\n"));
						PRINTF(LITERAL("     FW Version : None\n"));
						PRINTF(LITERAL("     PCI Subsystem Vendor ID : None\n"));
						PRINTF(LITERAL("     PCI Subsystem ID : None\n"));
					}
					else{
						PRINTF(LITERAL("Can't get device information! (%d)\n"), xhcsts);
						err++;
					}

					// Close Device
					xHcCloseDevice(g_DevInfo[i+g_DevInfoOffset].DevAddr);

					continue;
				}

				for(romLoop = 0; romLoop < SROM_SIDE_ALL; romLoop++){
					// FW Version
					if((devInfo.FwVersion[romLoop][0] == 0) &&
					   (devInfo.FwVersion[romLoop][1] == 0) &&
					   (devInfo.FwVersion[romLoop][2] == 0) &&
					   (devInfo.FwVersion[romLoop][3] == 0)
					   )
					{
						if(romLoop == SROM_SIDE_A){
							PRINTF(LITERAL("     FW Version : None\n"));
						}
						break;
					}
					else{
						PRINTF(LITERAL("     FW Version : %X.%X.%X.%X\n"),
											GetByteHi(devInfo.FwVersion[romLoop][3]),
											GetByteLo(devInfo.FwVersion[romLoop][3]),
											GetByteHi(devInfo.FwVersion[romLoop][2]),
											GetByteLo(devInfo.FwVersion[romLoop][2]));
					}

					// SubsystemVendorID SubSystemID
					if(devInfo.VcdInfo[romLoop] == FALSE){
						PRINTF(LITERAL("     None:ini file information is not found in ROM image.\n"));
					}
					else{
						if(devInfo.SSVIDInfo[romLoop] == FALSE){
							PRINTF(LITERAL("     PCI Subsystem Vendor ID : None:SSVID information is not found in ROM image\n"));
						}
						else{
							PRINTF(LITERAL("     PCI Subsystem Vendor ID : 0x%04X\n"), devInfo.SubSystemVendorID[romLoop]);
						}

						if(devInfo.SSIDInfo[romLoop] == FALSE){
							PRINTF(LITERAL("     PCI Subsystem ID : None:SSID information is not found in ROM image\n"));
						}
						else{
							PRINTF(LITERAL("     PCI Subsystem ID : 0x%04X\n"), devInfo.SubSystemID[romLoop]);
						}

						// EUI-64
						if(devInfo.EUIInfo[romLoop] == TRUE){
							PRINTF(LITERAL("     EUI-64 : %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n"),
												devInfo.Eui64[romLoop][0], devInfo.Eui64[romLoop][1], devInfo.Eui64[romLoop][2], devInfo.Eui64[romLoop][3],
												devInfo.Eui64[romLoop][4], devInfo.Eui64[romLoop][5], devInfo.Eui64[romLoop][6], devInfo.Eui64[romLoop][7]);
						}
					}
					printf("\n");
				}

			// Close Device
			xHcCloseDevice(g_DevInfo[i+g_DevInfoOffset].DevAddr);
			}
		}
		g_DevInfoOffset += i;

	}
	if(bRev){
		PRINTF(LITERAL("\nThis tool is for uPD720201(Revision %d & %d) and uPD720202(Revision %d).\n"),W201_TARGET_REVISION_2,W201_TARGET_REVISION_3, W202_TARGET_REVISION);
		PRINTF(LITERAL("Renesas Electronics release appropriate tool for the chip revision.\n"));
	}

	if(g_Command.srom == SROM_END){
		PRINTF(LITERAL("\n"));
		Usage(0,0);
	}
	if(err){
		ret = W201STS_ERROR;
		goto Exit_EnumDevice;
	}

Exit_EnumDevice:

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}

//
// /erase command
//
int EraseCommand(ULONG othercmd)
{
	unsigned long size;
	unsigned long writeSize = 0;
	int xhcsts;
	int ret;

	DebugTraceFunction(LITERAL(" ++\n"));

	// Device Open
	if((g_Command.cmd & W201CMD_WRITEALL) == 0){
		ret = DeviceOpen(FWREV_NOCHECK);
	}
	else{
		ret = W201STS_SUCCESS;
	}

	if(ret != W201STS_SUCCESS){
		goto Exit_EraseCommand;
	}

	// Memory allocate
	size = min(g_SRomInfo[g_Command.srom].Size, MAX_SROM_SIZE);

	xhcsts = xHcEraseSerialROM(g_DevAddr, 0);

	if(xhcsts != XHCSTS_SUCCESS){
		PRINTF(LITERAL("Serial ROM erase error (%d)\n"), xhcsts);
		ret = W201STS_DEVACCESS_ERR;
		goto Exit_EraseCommand;
	}

	PRINTF(LITERAL("Erase Serial ROM....  0%% "));

	do{
		SLEEP(WRITE_POLL_INTERVAL);

		// SROM write status
		xhcsts = xHcGetWriteStatus(g_DevAddr,	&writeSize);

		if(xhcsts < XHCSTS_SUCCESS){
			PRINTF(LITERAL("\nSerial ROM erase error (%d)\n"), xhcsts);
			ret = W201STS_DEVACCESS_ERR;
			goto Exit_EraseCommand;
		}

		PRINTF(LITERAL("\rErase Serial ROM....  %3ld%% "),	((writeSize * 100) / size));
	}while(xhcsts == XHCSTS_PROC_ACTIVE);

	ret = W201STS_SUCCESS;

	PRINTF(LITERAL("\rErase Serial ROM....  100%%\n"));
	PRINTF(LITERAL("Erase complete\n"));

Exit_EraseCommand:

	if(g_SelDev >= 0){
		if(othercmd == 0){
			// Device Close
			DeviceClose(g_DevAddr);
		}
	}

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);
	
	return ret;
}


//
// write command
//
int WriteCommand(void)
{
	FILE *fp = NULL;
	unsigned char* pBuf = NULL;
	unsigned long size;
	unsigned long writeSize = 0;
	LPCTSTR cfgpath, euipath;
	unsigned long sromRetCode;
	int xhcsts;
	int ret;
	unsigned char pFWVer[2];
	unsigned char fwRev = FWREV_NOCHECK;

	DebugTraceFunction(LITERAL(" ++\n"));

	// Device Open (& Get SROM info)
	ret = DeviceOpen(FWREV_NOCHECK);

	if(ret != W201STS_SUCCESS){
		goto Exit_WriteCommand;
	}

	size = min(g_SRomInfo[g_Command.srom].Size, MAX_SROM_SIZE);

	// Memory allocate
	pBuf = (UCHAR*)malloc(size);
	
	if(!pBuf){
		PRINTF(LITERAL("Memory allocate error = %ld\n"),size);
		ret = W201STS_MEMORY_ERR;
		goto Exit_WriteCommand;
	}

	if(g_Command.fname2[0] != LITERAL('\0')){
		cfgpath = g_Command.fname2;
	}
	else{
		cfgpath = NULL;
	}

	if(g_Command.fname3[0] != LITERAL('\0')){
		euipath = g_Command.fname3;
	}
	else{
		euipath = NULL;
	}

	// Make Image
	xhcsts = xHcUpdateROMImageFromFile(
						g_Command.fname1,
						cfgpath,
						euipath,
						&g_SRomInfo[g_Command.srom],
						pBuf
						);

	if(xhcsts != XHCSTS_SUCCESS){
		PRINTF(LITERAL("Update SROM Image error (%d)\n"), xhcsts);
		ret = ConvertStatus(xhcsts);
		goto Exit_WriteCommand;
	}

	// Get FW Version
	xhcsts = GetFwVersion(g_Command.fname1, pFWVer);
	fwRev  = GetByteHi(pFWVer[1]);

	// HW Revision Check
	if((g_DevInfo[g_SelDev].RevisionID == W201_TARGET_REVISION_2) || 
	   (g_DevInfo[g_SelDev].RevisionID == W201_TARGET_REVISION_3) || 
	   (g_DevInfo[g_SelDev].RevisionID == W202_TARGET_REVISION)){
		if((g_Command.fname1[0] == LITERAL('K')) || (g_Command.fname1[0] == LITERAL('k'))){
			if(g_findDevFlag == DEVICE_ID_TYPE_D201){
				if((fwRev != W201_TARGET_REVISION_2) && (fwRev != W201_TARGET_REVISION_3)){
					PRINTF(LITERAL("This chip revision is %x.\n"), g_DevInfo[g_SelDev].RevisionID);
					PRINTF(LITERAL("SROM ImageFile name is mismatch.\n"));
					ret = W201STS_REVISION_ERR;
					goto Exit_WriteCommand;
				}
			}
			else{
				if(fwRev != W202_TARGET_REVISION){
					PRINTF(LITERAL("This chip revision is %x.\n"), g_DevInfo[g_SelDev].RevisionID);
					PRINTF(LITERAL("SROM ImageFile name is mismatch.\n"));
					ret = W201STS_REVISION_ERR;
					goto Exit_WriteCommand;
				}
			}
		}
		else{
			PRINTF(LITERAL("HW Revision error\n"));
			ret = W201STS_REVISION_ERR;
			goto Exit_WriteCommand;
		}
	}
	else{
		PRINTF(LITERAL("HW Revision error.\n"));
		ret = W201STS_REVISION_ERR;
		goto Exit_WriteCommand;
	}

	// Flash Memory -> Erase
	if((g_Command.cmd & W201CMD_NOERASE) == 0){
		ret = EraseCommand(g_Command.cmd & W201CMD_ERSMASK2);
	}

	if(ret != W201STS_SUCCESS){
		goto Exit_WriteCommand;
	}

	xHcSetExtROMAccessEnable(g_DevInfo[g_SelDev].DevAddr, (USHORT)XHC_EXTROM_ACCESS_ENABLE);

	xhcsts = xHcWriteSerialROM( g_DevAddr,
								size,
								pBuf,
								XHC_NO_WAIT
								);

	if(xhcsts != XHCSTS_SUCCESS){
		PRINTF(LITERAL("Serial ROM write error (%d)\n"), xhcsts);
		ret = ConvertStatus(xhcsts);
		goto Exit_WriteCommand;
	}

	PRINTF(LITERAL("Write Serial ROM....  0%% "));

	do{
		SLEEP(WRITE_POLL_INTERVAL);

		// SROM write status
		xhcsts = xHcGetWriteStatus(g_DevAddr,	&writeSize);

		if(xhcsts < XHCSTS_SUCCESS){
			PRINTF(LITERAL("\nSerial ROM write error (%d)\n"), xhcsts);
			ret = W201STS_DEVACCESS_ERR;
			goto Exit_WriteCommand;
		}

		PRINTF(LITERAL("\rWrite Serial ROM....  %3ld%% "),	((writeSize * 100) / size));

	}while(xhcsts == XHCSTS_PROC_ACTIVE);

	PRINTF(LITERAL("\nWrite complete\n"));

	xHcSetExtROMAccessEnable(g_DevInfo[g_SelDev].DevAddr, (USHORT)XHC_EXTROM_ACCESS_DISABLE);
	while(1){
		xHcGetROMWriteResultCode(g_DevInfo[g_SelDev].DevAddr, &sromRetCode);

		if(sromRetCode == PCI_REG_RESULT_CODE_SUCCESS){
			DebugTrace(LITERAL("Serial ROM ResultCode :: Success\n"));
			break;
		}
		else if(sromRetCode == PCI_REG_RESULT_CODE_ERROR){
			DebugTrace(LITERAL("Serial ROM ResultCode :: Error\n"));
			break;
		}
		else{
			;// In case of PCI_REG_RESULT_CODE_INVALID, go on sromRetCode to acquire it again
		}

		SLEEP(READ_POLL_INTERVAL);
	}

	// verify

	if(sromRetCode == PCI_REG_RESULT_CODE_SUCCESS){
		PRINTF(LITERAL("Verify Serial ROM....  100%%\n"));
		PRINTF(LITERAL("Verify complete\n"));

//  Note:
//  Reload bit must be set after HCHalted(HCH) bit in 
//  USBSTS register is cleared. If the Reload bit is set
//  before HCH bit is cleard, the behavior is undefined.
//  
//  If you want to use Reload function(xHcResetDevice), you need to modify "XHCI_MAX_HALT_USEC" of "xhci-ext-caps.h".
//  (Before) #define XHCI_MAX_HALT_USEC	(16*125)
//  (After) #define XHCI_MAX_HALT_USEC	(16*1000)
//
//		ret = xHcResetDevice(g_DevInfo[g_SelDev].DevAddr);
//
	}
	else if(sromRetCode == PCI_REG_RESULT_CODE_ERROR){
		PRINTF(LITERAL("ERROR::Verify mismatch!\n"));
	}
	else{
		;
	}

Exit_WriteCommand:
	if((xhcsts != XHCSTS_SUCCESS) && (ret != W201STS_SUCCESS)){
		xHcSetExtROMAccessEnable(g_DevInfo[g_SelDev].DevAddr, (USHORT)XHC_EXTROM_ACCESS_DISABLE);
	}

	if(g_SelDev >= 0){
		// Device Close
		DeviceClose(g_DevAddr);
	}

	if(fp){
		fclose(fp);
	}

	if(pBuf){
		free(pBuf);
	}

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}

//
// /dump command
//
int DumpCommand(void)
{
	FILE *fp = NULL;
	unsigned char* pBuf = NULL;
	unsigned long size;
	unsigned long readSize;
	errno_t err;
	int xhcsts;
	int ret;

	DebugTraceFunction(LITERAL(" ++\n"));

	// Device Open
	ret = DeviceOpen(FWREV_NOCHECK);
	
	if(ret != W201STS_SUCCESS){
		goto Exit_DumpCommand;
	}

	// Memory allocate
	switch((g_setSromParam & SROM_ERASE_BLOCK_MASK) >> SROM_ERASE_BLOCK_SHIFT_BIT)
	{
		case SROM_EBSZ_4KBYTE:
			size = SROM_ERASE_BLOCK_SIZE_4K*SROM_READSIZE_EBSZ_4BYTE;
			break;
		case SROM_EBSZ_32KBYTE:
			size = SROM_ERASE_BLOCK_SIZE_32K*SROM_READSIZE;
			break;
		case SROM_EBSZ_64KBYTE:
			size = SROM_ERASE_BLOCK_SIZE_64K*SROM_READSIZE;
			break;
		case SROM_EBSZ_128KBYTE:
			size = SROM_ERASE_BLOCK_SIZE_128K*SROM_READSIZE;
			break;
		default:
			size = SROM_ERASE_BLOCK_SIZE_32K*SROM_READSIZE;
			break;
	}

	pBuf = (UCHAR*)malloc(size);
	if(!pBuf){
		PRINTF(LITERAL("Memory allocate error = %ld\n"),size);
		ret = W201STS_MEMORY_ERR;
		goto Exit_DumpCommand;
	}

	xHcSetExtROMAccessEnable(g_DevInfo[g_SelDev].DevAddr, (USHORT)XHC_EXTROM_ACCESS_ENABLE);

	// SROM read request
	xhcsts = xHcReadSerialROM(g_DevAddr,
							  0,
							  size,
							  NULL,
							  NULL,
							  XHC_NO_WAIT);

	if(xhcsts != XHCSTS_SUCCESS){
        PRINTF(LITERAL ("Serial ROM read error (%d)\n"), xhcsts);
		ret = W201STS_DEVACCESS_ERR;
		goto Exit_DumpCommand;
	}

	PRINTF(LITERAL("Read Serial ROM....  0%% "));

	do{
		SLEEP(READ_POLL_INTERVAL);

		// SROM read data
		xhcsts = xHcGetReadData(g_DevAddr,
								size,
								pBuf,
								&readSize);

		if(xhcsts < XHCSTS_SUCCESS){
			PRINTF(LITERAL("\nSerial ROM read error (%d)\n"), xhcsts);
			ret = W201STS_DEVACCESS_ERR;
			goto Exit_DumpCommand;
		}

		PRINTF(LITERAL("\rRead Serial ROM....  %3ld%% "),	((readSize * 100) / size));

	}while(xhcsts == XHCSTS_PROC_ACTIVE);

	PRINTF(LITERAL("\n"));

	// image file open
	err = FOPEN_S(&fp, g_Command.fname5, LITERAL("wb"));
	
	if (err) {
		PRINTF(LITERAL("Image file open error: %s(wb) (%d)\n"), g_Command.fname5, err);
		ret = W201STS_IMGFILE_ERR;
		goto Exit_DumpCommand;
	}
	
	if(fwrite(pBuf, size, 1, fp) != 1){
		PRINTF(LITERAL("Image file write error\n"));
		ret = W201STS_IMGFILE_ERR;
		goto Exit_DumpCommand;
	}

	ret = W201STS_SUCCESS;

	PRINTF(LITERAL("Dump complete\n"));

Exit_DumpCommand:
	xHcSetExtROMAccessEnable(g_DevInfo[g_SelDev].DevAddr, (USHORT)XHC_EXTROM_ACCESS_DISABLE);

	if(g_SelDev >= 0){
		// Device Close
		DeviceClose(g_DevAddr);
	}

	if(fp){
		fclose(fp);
	}

	if(pBuf){
		free(pBuf);
	}

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}

//
// verify command
//
int VerifyCommand(void)
{
	unsigned char* pBuf = NULL;
	unsigned long size;
	LPCTSTR cfgpath, euipath;
	int xhcsts;
	int ret;
	unsigned char pFWVer[2];
	unsigned char fwRev = FWREV_NOCHECK;

	DebugTraceFunction(LITERAL(" ++\n"));

	// Device Open (& Get SROM info)
	ret = DeviceOpen(FWREV_NOCHECK);

	if(ret != W201STS_SUCCESS){
		goto Exit_VerifyCommand;
	}

	size = min(g_SRomInfo[g_Command.srom].Size, MAX_SROM_SIZE);

	// Memory allocate
	pBuf = (UCHAR*)malloc(size);
	
	if(!pBuf){
		PRINTF(LITERAL("Memory allocate error = %ld\n"),size);
		ret = W201STS_MEMORY_ERR;
		goto Exit_VerifyCommand;
	}

	if(g_Command.fname2[0] != LITERAL('\0')){
		cfgpath = g_Command.fname2;
	}
	else{
		cfgpath = NULL;
	}

	if(g_Command.fname3[0] != LITERAL('\0')){
		euipath = g_Command.fname3;
	}
	else{
		euipath = NULL;
	}

	// Make Image
	xhcsts = xHcUpdateROMImageFromFile(
						g_Command.fname1,
						cfgpath,
						euipath,
						&g_SRomInfo[g_Command.srom],
						pBuf
						);

	if(xhcsts != XHCSTS_SUCCESS){
		PRINTF(LITERAL("Update SROM Image error (%d)\n"), xhcsts);
		ret = ConvertStatus(xhcsts);
		goto Exit_VerifyCommand;
	}

	// Get FW Version
	xhcsts = GetFwVersion(g_Command.fname1, pFWVer);
	fwRev = GetByteHi(pFWVer[1]);

	// HW Revision Check
	if((g_DevInfo[g_SelDev].RevisionID == W201_TARGET_REVISION_2) || 
	   (g_DevInfo[g_SelDev].RevisionID == W201_TARGET_REVISION_3) || 
	   (g_DevInfo[g_SelDev].RevisionID == W202_TARGET_REVISION)){
		if((g_Command.fname1[0] == LITERAL('K')) || (g_Command.fname1[0] == LITERAL('k'))){
			if(g_findDevFlag == DEVICE_ID_TYPE_D201){
				if((fwRev != W201_TARGET_REVISION_2) && (fwRev != W201_TARGET_REVISION_3)){
					PRINTF(LITERAL("This chip revision is %x.\n"), g_DevInfo[g_SelDev].RevisionID);
					PRINTF(LITERAL("SROM ImageFile name is mismatch.\n"));
					ret = W201STS_REVISION_ERR;
					goto Exit_VerifyCommand;
				}
			}
			else{
				if(fwRev != W202_TARGET_REVISION){
					PRINTF(LITERAL("This chip revision is %x.\n"), g_DevInfo[g_SelDev].RevisionID);
					PRINTF(LITERAL("SROM ImageFile name is mismatch.\n"));
					ret = W201STS_REVISION_ERR;
					goto Exit_VerifyCommand;
				}
			}
		}
		else{
			PRINTF(LITERAL("HW Revision error\n"));
			ret = W201STS_REVISION_ERR;
			goto Exit_VerifyCommand;
		}
	}
	else{
		PRINTF(LITERAL("HW Revision error.\n"));
		ret = W201STS_REVISION_ERR;
		goto Exit_VerifyCommand;
	}

	switch((g_setSromParam & SROM_ERASE_BLOCK_MASK) >> SROM_ERASE_BLOCK_SHIFT_BIT){
		case SROM_EBSZ_4KBYTE:
			size = SROM_ERASE_BLOCK_SIZE_4K*4;
			break;
		default:
			size = SROM_ERASE_BLOCK_SIZE_32K;
			break;
	}

	// verify
	ret = SRomVerify(pBuf, size);

Exit_VerifyCommand:

	if(g_SelDev >= 0){
		// Device Close
		DeviceClose(g_DevAddr);
	}

	if(pBuf){
		free(pBuf);
	}

	DebugTraceFunction(LITERAL(" -- (%d)\n"),ret);

	return ret;
}

// writeall command
//
int WriteAllCommand(void)
{
	unsigned char* pBuf = NULL;
	unsigned long size;
	unsigned long writeSize = 0;
	LPCTSTR cfgpath, euipath;
	XHC_SROM_INFO *pSromInfo;
	unsigned long sromRetCode;
	int loop;
	int findAll = 0;
	int num = 0;
	int i;
	int xhcsts;
	int ret;
	unsigned char fwRev = FWREV_NOCHECK;
	unsigned char pFWVer[2];

	DebugTraceFunction(LITERAL(" ++\n"));

	g_DevInfoOffset = 0;
	for(loop = 0; loop < DEVICE_ID_TYPE_ALL; loop++){
		g_findDevFlag = loop;
		ret = FindDevice();

		if(ret != W201STS_SUCCESS){
			goto Exit_WriteAllCommand;
		}

		if(g_FindNum == 0){
			if(loop == DEVICE_ID_TYPE_D201){
				continue;
			}
			else{
				if(findAll == FALSE){
					PRINTF(LITERAL("Device not found!\n"));
					ret = W201STS_DEVICE_FOUND_ERR;
					goto Exit_WriteAllCommand;
				}
				else{
					ret = W201STS_SUCCESS;
					goto Exit_WriteAllCommand;
				}
			}
		}

		findAll += g_FindNum;

		if(g_Command.srom == SROM_AUTO){
			size = MAX_SROM_SIZE;

			g_SRomInfo[SROM_AUTO].Size = size;
			pSromInfo = &g_SRomInfo[SROM_AUTO];
		}
		else{
			size = min(g_SRomInfo[g_Command.srom].Size, MAX_SROM_SIZE);

			pSromInfo = &g_SRomInfo[g_Command.srom];
		}

		// Memory allocate
		pBuf = (UCHAR*)malloc(size);
		if(!pBuf){
			PRINTF(LITERAL("Memory allocate error = %ld\n"),size);
			ret = W201STS_MEMORY_ERR;
			goto Exit_WriteAllCommand;
		}

		if(g_Command.fname2[0] != LITERAL('\0')){
			cfgpath = g_Command.fname2;
		}
		else{
			cfgpath = NULL;
		}

		if(g_Command.fname3[0] != LITERAL('\0')){
			euipath = g_Command.fname3;
		}
		else{
			euipath = NULL;
		}

		// Make Image
		xhcsts = xHcUpdateROMImageFromFile(
							g_Command.fname1,
							cfgpath,
							euipath,
							pSromInfo,
							pBuf
							);

		if(xhcsts != XHCSTS_SUCCESS){
			PRINTF(LITERAL("Update SROM Image error (%d)\n"), xhcsts);
			ret = ConvertStatus(xhcsts);
			goto Exit_WriteAllCommand;
		}

		// Get FW Version
		xhcsts = GetFwVersion(g_Command.fname1, pFWVer);
		fwRev = GetByteHi(pFWVer[1]);

		for(i=0; i<(findAll - g_DevInfoOffset); i++){
			// HW Revision Check
			if((g_DevInfo[i+g_DevInfoOffset].RevisionID == W201_TARGET_REVISION_2) || 
			   (g_DevInfo[i+g_DevInfoOffset].RevisionID == W201_TARGET_REVISION_3) || 
			   (g_DevInfo[i+g_DevInfoOffset].RevisionID == W202_TARGET_REVISION)){
				if((g_Command.fname1[0] == LITERAL('K')) || (g_Command.fname1[0] == LITERAL('k'))){
					if(g_findDevFlag == DEVICE_ID_TYPE_D201){
						if((fwRev != W201_TARGET_REVISION_2) & (fwRev != W201_TARGET_REVISION_3)){
							PRINTF(LITERAL("This chip revision is %x.\n"), g_DevInfo[i+g_DevInfoOffset].RevisionID);
							PRINTF(LITERAL("SROM ImageFile name is mismatch.\n"));
							ret = W201STS_REVISION_ERR;
							goto Exit_WriteAllCommand;
						}
					}
					else{
						if(fwRev != W202_TARGET_REVISION){
							PRINTF(LITERAL("This chip revision is %x.\n"), g_DevInfo[i+g_DevInfoOffset].RevisionID);
							PRINTF(LITERAL("SROM ImageFile name is mismatch.\n"));
							ret = W201STS_REVISION_ERR;
							goto Exit_WriteAllCommand;
						}
					}
				}
				else{
					PRINTF(LITERAL("HW Revision error\n"));
					ret = W201STS_REVISION_ERR;
					goto Exit_WriteAllCommand;
				}
			}
			else{
				PRINTF(LITERAL("HW Revision error.\n"));
				ret = W201STS_REVISION_ERR;
				goto Exit_WriteAllCommand;
			}

			PRINTF(LITERAL("[Target Device]Bus:0x%02X Device:0x%02X Function:0x%02X\n"),
								GetBus(g_DevInfo[i+g_DevInfoOffset].DevAddr),
								GetDev(g_DevInfo[i+g_DevInfoOffset].DevAddr),
								GetFunc(g_DevInfo[i+g_DevInfoOffset].DevAddr));
			if(g_findDevFlag == DEVICE_ID_TYPE_D201){
				PRINTF(LITERAL("This Device is uPD720201(Revision %d).\n"),g_DevInfo[i+g_DevInfoOffset].RevisionID);
			}
			else if(g_findDevFlag == DEVICE_ID_TYPE_D202){
				PRINTF(LITERAL("This Device is uPD720202(Revision %d).\n"),g_DevInfo[i+g_DevInfoOffset].RevisionID);
			}

			num++;
			g_DevAddr = g_DevInfo[i+g_DevInfoOffset].DevAddr;

			xhcsts = xHcOpenDevice(
								g_DevInfo[i+g_DevInfoOffset].DevAddr,
								XHC_OPMODE_UFW,
								NULL,
								g_DevInfo[i+g_DevInfoOffset].RevisionID
								);

			if(xhcsts != XHCSTS_SUCCESS){
				PRINTF(LITERAL("Can't open device! (%d)\n"), xhcsts);
				ret = W201STS_ERROR;
				goto Exit_WriteAllCommand;
			}

			g_SelDev = i+g_DevInfoOffset;

			// SROM Info Check
			ret = GetSromInfo(g_DevInfo[i+g_DevInfoOffset].DevAddr, FALSE);
			if(ret != W201STS_SUCCESS){
				goto Exit_WriteAllCommand;
			}

			if((g_Command.cmd & W201CMD_NOERASE) == 0){
				ret = EraseCommand(g_Command.cmd & W201CMD_ERSMASK2);
			}

			if(ret != W201STS_SUCCESS){
				goto Exit_WriteAllCommand;
			}

			xHcSetExtROMAccessEnable(g_DevInfo[i+g_DevInfoOffset].DevAddr, (USHORT)XHC_EXTROM_ACCESS_ENABLE);
			xhcsts = xHcWriteSerialROM(
							g_DevAddr,
							size,
							pBuf,
							XHC_NO_WAIT
							);

			if(xhcsts != XHCSTS_SUCCESS){
				PRINTF(LITERAL("Serial ROM write error (%d)\n"), xhcsts);
				ret = ConvertStatus(xhcsts);
				goto Exit_WriteAllCommand;
			}

			PRINTF(LITERAL("Write Serial ROM....  0%% "));
			do{
				SLEEP(WRITE_POLL_INTERVAL);

				// SROM write status
				xhcsts = xHcGetWriteStatus(g_DevAddr,	&writeSize);
										 
				if(xhcsts < XHCSTS_SUCCESS){
					PRINTF(LITERAL("\nSerial ROM write error (%d)\n"), xhcsts);
					ret = W201STS_DEVACCESS_ERR;
					goto Exit_WriteAllCommand;
				}

				PRINTF(LITERAL("\rWrite Serial ROM....  %3ld%% "),	((writeSize * 100) / size));
			}while(xhcsts == XHCSTS_PROC_ACTIVE);

			PRINTF(LITERAL("\nWrite complete\n"));

			xHcSetExtROMAccessEnable(g_DevInfo[i+g_DevInfoOffset].DevAddr, (USHORT)XHC_EXTROM_ACCESS_DISABLE);
			while(1){
				xHcGetROMWriteResultCode(g_DevInfo[i+g_DevInfoOffset].DevAddr, &sromRetCode);

				if(sromRetCode == PCI_REG_RESULT_CODE_SUCCESS){
					DebugTrace(LITERAL("Serial ROM ResultCode :: Success\n"));
					break;
				}
				else if(sromRetCode == PCI_REG_RESULT_CODE_ERROR){
					DebugTrace(LITERAL("Serial ROM ResultCode :: Error\n"));
					break;
				}
				else{
					;// In case of PCI_REG_RESULT_CODE_INVALID, go on sromRetCode to acquire it again
				}

				SLEEP(READ_POLL_INTERVAL);
			}

			// verify
			if(sromRetCode == PCI_REG_RESULT_CODE_SUCCESS){
				PRINTF(LITERAL("Verify Serial ROM....  100%%\n"));
				PRINTF(LITERAL("Verify complete\n"));

//  Note:
//  Reload bit must be set after HCHalted(HCH) bit in 
//  USBSTS register is cleared. If the Reload bit is set
//  before HCH bit is cleard, the behavior is undefined.
//  
//  If you want to use Reload function(xHcResetDevice), you need to modify "XHCI_MAX_HALT_USEC" of "xhci-ext-caps.h".
//  (Before) #define XHCI_MAX_HALT_USEC	(16*125)
//  (After) #define XHCI_MAX_HALT_USEC	(16*1000)
//
//				ret = xHcResetDevice(g_DevInfo[i+g_DevInfoOffset].DevAddr);
//
			}
			else if(sromRetCode == PCI_REG_RESULT_CODE_ERROR){
				PRINTF(LITERAL("ERROR::Verify mismatch!\n"));
			}
			else{
				;
			}

			// Close Device
			xHcCloseDevice(g_DevInfo[i+g_DevInfoOffset].DevAddr);
			g_SelDev = -1;
		}
		g_DevInfoOffset += i;

		if(loop > DEVICE_ID_TYPE_D201){
			if(!num){
				PRINTF(LITERAL("Device not found!\n"));
				ret = W201STS_DEVICE_FOUND_ERR;
				goto Exit_WriteAllCommand;
			}
		}
	}
Exit_WriteAllCommand:
	if((xhcsts != XHCSTS_SUCCESS) && (ret != W201STS_SUCCESS) && (ret != W201STS_DEVICE_FOUND_ERR)){
		xHcSetExtROMAccessEnable(g_DevInfo[g_SelDev].DevAddr, (USHORT)XHC_EXTROM_ACCESS_DISABLE);
	}

	// Close Device
	if(g_SelDev != -1){
		xHcCloseDevice(g_DevInfo[g_SelDev].DevAddr);
	}

	if(pBuf){
		free(pBuf);
	}

	DebugTrace(LITERAL(" -- (%d)\n"),ret);

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//		Sub Function
///////////////////////////////////////////////////////////////////////////////
int GetFwVersion(LPCTSTR fname, unsigned char* fwRev)
{
	errno_t err;
	FILE* ImgFile = NULL;
	unsigned char pFwAdder[4];

	err = FOPEN_S(&ImgFile, fname, LITERAL("rb"));

	if (err) {
		ImgFile = NULL;
		DebugTrace (LITERAL("ROM Image file open error: %s(rb) (%d)\n"), fname, err);
		return W201STS_FILE_NOT_FOUND;
	}

	// Get Boot FW Address
	if(fseek(ImgFile, XHC_SROM_ACCESS_SIZE, SEEK_SET)){
		DebugTrace (LITERAL("ROM Image file seek errorqw!\n"));
		return W201STS_FILE_NOT_FOUND;
	}

	fread(pFwAdder, 1, XHC_SROM_ACCESS_SIZE_SHORT, ImgFile);

	// Get FW Version
	if(fseek(ImgFile, (long)*pFwAdder, SEEK_SET)){
		PRINTF(LITERAL("ROM Image file seek error!\n"));
		return W201STS_FILE_NOT_FOUND;
	}

	fread(fwRev, 1, XHC_SROM_ACCESS_SIZE_SHORT, ImgFile);

	FCLOSE (ImgFile);
	ImgFile = NULL;


	return W201STS_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//		Main Function
///////////////////////////////////////////////////////////////////////////////


// 
// Main
//
int MAIN(int argc, LITERALCHAR* argv[])
{
	BOOL bInit = FALSE;
	int i;
	int logret = W201STS_ERROR;
	int xhcsts;
	int ret;

	DebugTrace(LITERAL("Application Start\n"));
	//DebugPause();

	// command line
	strcpy(g_AppName, argv[0]);
	g_CommandLine[0] = LITERAL('\0');
	for(i=0;i<argc;i++){
		strcat(g_CommandLine, argv[i]);
		strcat(g_CommandLine, LITERAL(" "));
	}
	g_CommandLine[STRLEN(g_CommandLine) - 1] = LITERAL('\n');

	// command line check
	ret = CheckCommandLine(argc,argv);
	
	if(ret != W201STS_SUCCESS){
		goto Exit_main;
	}

	// Usage
	if(g_Command.cmd & W201CMD_USAGE){
		Usage(0,1);
		ret = W201STS_SUCCESS;
		goto Exit_main;
	}
	// Extended Usage
	else if(g_Command.cmd & W201CMD_EXUSAGE){
		Usage(1,1);
		ret = W201STS_SUCCESS;
		goto Exit_main;
	}
	
	// RUN command not found
	if((g_Command.cmd & W201CMD_EXMASK) == 0){
		goto Exit_main;
	}

	// library init
	xhcsts = xHcInitialize();

	if(xhcsts != XHCSTS_SUCCESS){
		PRINTF(LITERAL("xhc201 Library Initialize Error! (%d)\n"), xhcsts);
		ret = W201STS_ERROR;
		goto Exit_main;
	}

	bInit = TRUE;

	DebugTrace(LITERAL("Command Execute\n"));

	// Log Create
	if(g_Command.cmd & W201CMD_LOG){
		logret = OpenLogFile(g_Command.fname4);
	}

	if(g_Command.cmd & W201CMD_WRITE){
		ret = WriteCommand();
	}
	else if(g_Command.cmd & W201CMD_ERASE){
		ret = EraseCommand(0);
	}
	else if(g_Command.cmd & W201CMD_DUMP){
		ret = DumpCommand();
	}
	else if(g_Command.cmd & W201CMD_VERIFY){
		ret = VerifyCommand();
	}
	else if(g_Command.cmd & W201CMD_WRITEALL){
		ret = WriteAllCommand();
	}
	else if(g_Command.cmd & W201CMD_ENUM){
		ret = EnumDevice();
	}

Exit_main:

	if(bInit){
		xhcsts = xHcTerminate();

		if(xhcsts != XHCSTS_SUCCESS){
			PRINTF(LITERAL("xhc201w Library Terminate Error! (%d)\n"), xhcsts);

			if(ret == W201STS_SUCCESS){
				ret = W201STS_ERROR;
			}
		}
	}

	if(logret == W201STS_SUCCESS){

		// Log Write
		LogWrite(g_CommandLine,g_DevAddr,ret);

		CloseLogFile();
	}

	if(g_DevInfo){
		free(g_DevInfo);
		g_DevInfo = NULL;
	}

	//DebugPause();

	DebugTrace(LITERAL("Application Exit (%d)\n"),ret);

	return ret;
}

