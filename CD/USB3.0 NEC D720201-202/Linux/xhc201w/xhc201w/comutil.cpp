/*
	File Name : comutil.h
	
	Common Utility

	* Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2009-10-5	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////
#include "define.h"
#include "xhc201w.h"
#include "comutil.h"


//********************************************************************************//
//	Global
//********************************************************************************//

// Status Convert Table
static STATUS_CONV_ENTRY g_StatusConvTable[] =
{
	// Err								XHC Status					 NTSTATUS
	{ ERROR_SUCCESS,					XHCSTS_SUCCESS			},	// STATUS_SUCCESS					(0x00000000)
	{ ERROR_IO_DEVICE,					XHCSTS_ERROR			},	// STATUS_IO_DEVICE_ERROR			(0xC0000185)
	{ ERROR_INVALID_FUNCTION,			XHCSTS_ERROR			},	// STATUS_INVALID_DEVICE_REQUEST	(0xC0000010)
	{ ERROR_INVALID_HANDLE,				XHCSTS_NOT_OPENED		},	// STATUS_INVALID_HANDLE			(0xC0000008)
	{ ERROR_BAD_COMMAND,				XHCSTS_INVALID_REQUEST	},	// STATUS_INVALID_DEVICE_STATE		(0xC0000184)
	{ ERROR_INVALID_PARAMETER,			XHCSTS_PARAMETER_ERROR	},	// STATUS_INVALID_PARAMETER			(0xC000000D)
	{ ERROR_INSUFFICIENT_BUFFER,		XHCSTS_MEMORY_ERROR		},	// STATUS_BUFFER_TOO_SMALL			(0xC0000023)
	{ ERROR_BUSY,						XHCSTS_BUSY				},	// STATUS_DEVICE_BUSY				(0x80000011)
	{ ERROR_NO_SYSTEM_RESOURCES,		XHCSTS_ERROR			},	// STATUS_INSUFFICIENT_RESOURCES	(0xC000009A)
	{ ERROR_DEVICE_ALREADY_ATTACHED,	XHCSTS_ALREADY_OPEN		},	// STATUS_DEVICE_ALREADY_ATTACHED	(0xC0000038)
	{ ERROR_NOT_READY,					XHCSTS_XHCIRST_ERROR	},	// STATUS_DEVICE_NOT_READY			(0xC00000A3)
	{ ERROR_NO_MATCH,					XHCSTS_UNKNOWN_SROM		}	// STATUS_NO_MATCH					(0xC0000272)
	
};

#ifdef _DEBUG
UINT g_MallocCount = 0;
#endif

//********************************************************************************//
//	Function
//********************************************************************************//


// Convert Error Code to xhc status
int ConvertStatus(DWORD dwRet)
{
	int i,count;

	count = sizeof(g_StatusConvTable) / sizeof(g_StatusConvTable[0]);
	
	for(i=0;i<count;i++){


		if(dwRet == g_StatusConvTable[i].ErrSts){
			return g_StatusConvTable[i].Xhcsts;
		}
	}

	return XHCSTS_ERROR;
}


#ifdef _DEBUG


void* xhcmalloc(size_t size){
	void* ptr;

	ptr = malloc(size);

	if(ptr){
		g_MallocCount++; 
	}

	return ptr;
}


void xhcfree(void* ptr){

	g_MallocCount--; 

	free(ptr);

}

#endif

