/*
	File Name : xhc201h.h
	
	Hide Export API for uPD720201

	* Copyright (C) 2010 Renesas Electronics Corporation
	* Copyright (c) 2009 NEC Engineering, Ltd.

*/

///////////////////////////////////////////////////////////////////////////////
//
//		History
//		2009-09-28	rev0.01		base create
//
///////////////////////////////////////////////////////////////////////////////


#ifndef _XHC201H_H_
#define _XHC201H_H_

#include "xhc201w.h"


#ifdef _cplusplus
extern "C" {
#endif // #ifdef _cplusplus


//********************************************************************************//
//	define
//********************************************************************************//


//********************************************************************************//
//	struct
//********************************************************************************//


//********************************************************************************//
//	macro
//********************************************************************************//



//********************************************************************************//
//	API
//********************************************************************************//

/*
	Common Function
*/



// Reset Target Device
XHC201W_API int _API xHcResetDevice(unsigned short devAddr);





#ifdef _cplusplus
}
#endif // #ifdef _cplusplus

#endif // #ifndef _XHC201H_H_

