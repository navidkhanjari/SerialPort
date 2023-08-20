/*
	File Name : fileio.h
	
	file I/O 

	* Copyright (C) 2010 Renesas Electronics Corporation
	* Copyright (c) 2009 NEC Engineering, Ltd.

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2009-10-10	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _FILEIO_H_
#define _FILEIO_H_



//********************************************************************************//
//	define
//********************************************************************************//

// line buffer size
#define LINE_BUFFER_SIZE	4096	// * sizeof(TCHAR)

//********************************************************************************//
//	Function
//********************************************************************************//

// ROM Image file open
int OpenImage(LPCTSTR fname,long *flength);
// ROM Image file close
void CloseImage(void);
// ROM Image Read 
int ReadImage(long offset,UCHAR* pBuf, size_t size);
// Load Image
int LoadSROMImage(LPCTSTR fname,
				  long size,
				  UCHAR *pBuf,
				  ULONG *pFileSize,
				  ULONG *pLoadSize
				  );


// FW file open
int OpenFwData(LPCTSTR fname,long *flength);
// FW file close
void CloseFwData(void);
// FW Read 
int ReadFwData(long offset,UCHAR *pBuf, size_t size);
// Load FW
int LoadFwData(LPCTSTR fname,
			   long size,
			   UCHAR *pBuf,
			   ULONG *pFileSize,
			   ULONG *pLoadSize
			  );


// Read EUI-64
int ReadEui64(LPCTSTR euiname, UCHAR* pEui64, UINT index);

// Read & analyze Config Data(D720201)
int ReadVendorSpecificConfigInfo(LPCTSTR cfgname, 
								 USHORT *pSubSysID,
								 USHORT *pSubSysVenID,
								 PXHC_CFG_SETTING_INFO pconfigSetting,
								 BOOL *fwupdate);

#endif // #ifndef _FILEIO_H_
