/*
    File Name : drvsvc.c

    Driver Service Control

    * Copyright (C) 2010 Renesas Electronics Corporation

*/

///////////////////////////////////////////////////////////////////////////////
//
//      History
//      2010-08-16  rev0.00     base create
//
///////////////////////////////////////////////////////////////////////////////


#include "define.h"
#include "xhc201w.h"
#include "comutil.h"
#include "drvsvc.h"


//********************************************************************************//
//  Global
//********************************************************************************//


//********************************************************************************//
//  Function
//********************************************************************************//

//--------------------------------------------------------------------------------
//  Service Function
//--------------------------------------------------------------------------------

BOOL DeleteServiceEx(SC_HANDLE hSCManager,SC_HANDLE& hService)
{

    SERVICE_STATUS svcStatus;
    SC_HANDLE hChkService;
    int i;
    BOOL bRet;
    DWORD dwRet;

    bRet = DeleteService(hService);

    if(!bRet){

        dwRet = GetLastError();

        DebugTrace(_T("DeleteService fail (%d)\n"),dwRet);

        // Already delete
        if(dwRet == ERROR_SERVICE_MARKED_FOR_DELETE){
            bRet = TRUE;
            // update service status
            ControlService(hService,SERVICE_CONTROL_INTERROGATE,&svcStatus);
        }
        else{
            CloseServiceHandle(hService);
            hService = NULL;
            return FALSE;
        }
    }

    CloseServiceHandle(hService);
    hService = NULL;

    // wait delete
    if(bRet){
        bRet = FALSE;

        DebugTrace(_T("Wait : service delete complete\n"));

        for(i=0;i<SERVICE_DELETE_WAITCOUNT;i++){

            Sleep(SERVICE_DELETE_WAITSLEEP);

            hChkService = OpenService(hSCManager,
                                      DRVSVC_NAME,
                                      SERVICE_ALL_ACCESS);
            if(!hChkService){
                DebugTrace(_T("Service delete completed\n"));
                bRet = TRUE;
                break;
            }

            QueryServiceStatus(hChkService, &svcStatus);

            DebugTrace(_T("Service status= 0x%08X\n"), svcStatus.dwCurrentState);

            CloseServiceHandle(hChkService);
        }
    }

    return bRet;

}


//
// Register & Start Service
//
BOOL RegisterService(LPCTSTR drvPath)
{
    BOOL bRet(FALSE);
    SC_HANDLE hSCManager(NULL);
    SC_HANDLE hService(NULL);
    LPQUERY_SERVICE_CONFIG pSvcConfig(NULL);
    SERVICE_STATUS svcStatus;
    DWORD dwLen(0);
    DWORD dwRet;
    BOOL bDelete(FALSE);
    BOOL bCreate(TRUE);

    CString tmpStr;
    CString devStr;

    int retry = 0;

    DebugTrace(_T("Register Service Start\n"));


    hSCManager = OpenSCManager(
                            NULL,                   // computer name
                            NULL,                   // SCM database name
                            SC_MANAGER_ALL_ACCESS   // access type
                            );

    if (!hSCManager)
    {
        DebugTrace(_T("OpenSCManager fail (%d)\n"),GetLastError());
        goto Exit_RegisterService;
    }



    hService = OpenService(hSCManager,
                           DRVSVC_NAME,
                           SERVICE_ALL_ACCESS);
    // Already Exist
    if(hService)
    {
        DebugTrace(_T("Service is already running ?\n"));

        bRet = TRUE;

        // Check Status
        bRet = QueryServiceStatus(hService, &svcStatus);

        if(!bRet){
            DebugTrace(_T("QueryServiceStatus fail (%d)\n"),GetLastError());
            goto Exit_RegisterService;
        }

        DebugTrace(_T("Service status= 0x%08X\n"), svcStatus.dwCurrentState);


        // Check Config
        while(!QueryServiceConfig(hService,pSvcConfig,dwLen,&dwLen)){

            dwRet = GetLastError();

            if(dwRet == ERROR_INSUFFICIENT_BUFFER){

                if(pSvcConfig){
                    delete [] pSvcConfig;
                    pSvcConfig = NULL;;
                }

                pSvcConfig = (LPQUERY_SERVICE_CONFIG)new UCHAR[dwLen];
            }
            else{
                bRet = FALSE;
                DebugTrace(_T("QueryServiceConfig fail (%d)\n"),dwRet);
                break;
            }
        }

        if(pSvcConfig){
            tmpStr = pSvcConfig->lpBinaryPathName;
            delete [] pSvcConfig;
        }

        if(bRet){
            devStr = drvPath;
            devStr.MakeLower();
            tmpStr.MakeLower();

            if(tmpStr.Find(devStr) == -1){
                bDelete = TRUE;
            }
            else{
                DebugTrace(_T("Driver is already registered  = %s\n"), drvPath);
                bCreate = FALSE;
            }
        }
        else{
            goto Exit_RegisterService;
        }

        // Already stopped
        if(bDelete){
            if(svcStatus.dwCurrentState == SERVICE_STOPPED){
                bRet = DeleteServiceEx(hSCManager,hService);
            }
            else{
                DebugTrace(_T("Other service is already running\n"));
                bRet = FALSE;
                goto Exit_RegisterService;
            }
        }
    }


    dwRet = ERROR_SUCCESS;

    while(retry < XHC201_REGSRV_RETRY){

        // Create Service
        if(bCreate){

            DebugTrace(_T("CreateService = %s\n"),DRVSVC_NAME);

            hService = CreateService(
                            hSCManager,         // SCManager database
                            DRVSVC_NAME,            // name of service
                            DRVSVC_NAME,            // name to display
                            SERVICE_ALL_ACCESS,     // desired access
                            SERVICE_KERNEL_DRIVER,  // service type
                            SERVICE_DEMAND_START,   // start type
                            SERVICE_ERROR_NORMAL,   // error control type
                            drvPath,                // service's binary
                            NULL,                   // no load ordering group
                            NULL,                   // no tag identifier
                            NULL,                   // no dependencies
                            NULL,                   // LocalSystem account
                            NULL                    // no password
                            );

            if(hService){
                bRet = TRUE;
            }
            else{

                dwRet = GetLastError();

                DebugTrace(_T("CreateService fail (%d)\n"),dwRet);

                if(dwRet == ERROR_SERVICE_EXISTS){
                    bRet = TRUE;
                }
                else{

                    hService = CreateService(
                                    hSCManager,         // SCManager database
                                    DRVSVC_NAME,            // name of service
                                    DRVSVC_NAME,            // name to display
                                    SERVICE_ALL_ACCESS,     // desired access
                                    SERVICE_KERNEL_DRIVER,  // service type
                                    SERVICE_DEMAND_START,   // start type
                                    SERVICE_ERROR_NORMAL,   // error control type
                                    drvPath,                // service's binary
                                    NULL,                   // no load ordering group
                                    NULL,                   // no tag identifier
                                    NULL,                   // no dependencies
                                    NULL,                   // LocalSystem account
                                    NULL                    // no password
                                    );

                    if(hService){
                        bRet = TRUE;
                    }
                    else{
                        dwRet = GetLastError();

                        DebugTrace(_T("CreateService (2) fail (%d)\n"),dwRet);

                        if(dwRet == ERROR_SERVICE_EXISTS){
                            bRet = TRUE;
                        }
                    }
                }
            }

        }


        // Start Service
        if(bRet){
            DebugTrace(_T("StartService = 0x%08X\n"),hService);

            bRet = StartService(hService, 0, NULL);

            if(!bRet){
                dwRet = GetLastError();

                if(dwRet == ERROR_SERVICE_ALREADY_RUNNING){
                    bRet = TRUE;
                }
                else{
                    dwRet = GetLastError();
                    DebugTrace(_T("StartService fail (%d)\n"),dwRet);
                }
            }
        }


        if((!bRet) && (dwRet == ERROR_SERVICE_DISABLED)){
            // delete & retry
            retry++;
            DeleteServiceEx(hSCManager,hService);
            bCreate = TRUE;

        }
        else{
            break;
        }
    }


Exit_RegisterService:

    if(hService){
        CloseServiceHandle(hService);
    }

    if(hSCManager){
        CloseServiceHandle(hSCManager);
    }

    if(bRet){
        DebugTrace(_T("Register Service Success\n"));
    }
    else{
        DebugTrace(_T("Register Service Fail\n"));
    }

    return bRet;

}


//
// Unregister Service
//
BOOL UnregisterService(void)
{
    BOOL bRet(FALSE);
    SC_HANDLE hSCManager(NULL);
    SC_HANDLE hService(NULL);
    SERVICE_STATUS svcStatus;

    CString tmpStr;

    DebugTrace(_T("Unregistered Service Start\n"));


    hSCManager = OpenSCManager(
                            NULL,                   // computer name
                            NULL,                   // SCM database name
                            SC_MANAGER_ALL_ACCESS   // access type
                            );

    if (!hSCManager)
    {
        DebugTrace(_T("OpenSCManager fail (%d)\n"),GetLastError());
        return FALSE;
    }



    hService = OpenService(hSCManager,
                           DRVSVC_NAME,
                           SERVICE_ALL_ACCESS);
    // Already Exist
    if(hService)
    {

        DebugTrace(_T("OpenService (0x%08X)\n"),hService);


        bRet = ControlService( hService,
                               SERVICE_CONTROL_STOP,
                               &svcStatus);


        if(!bRet){
            DebugTrace(_T("ControlService fail (%d)\n"),GetLastError());
        }

        bRet = DeleteService(hService);

        if(!bRet){
            DebugTrace(_T("DeleteService fail (%d)\n"),GetLastError());
        }

    }

    if(hService){
        CloseServiceHandle(hService);
    }

    if(hSCManager){
        CloseServiceHandle(hSCManager);
    }

    if(bRet){
        DebugTrace(_T("Unregistered Service Success\n"));
    }
    else{
        DebugTrace(_T("Unregistered Service Fail\n"));
    }

    return bRet;
}



//--------------------------------------------------------------------------------
//  Driver Function
//--------------------------------------------------------------------------------


//
// Open Driver
//
HANDLE OpenDriver(void)
{

    HANDLE hDrv;

    hDrv = CreateFile(
                (LPCTSTR)DRIVERSYMBOLNAME,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL );

    if(hDrv == INVALID_HANDLE_VALUE){
        DebugTrace(_T("CreateFile fail (%d)\n"),GetLastError());
        hDrv = NULL;
    }
    else{
        DebugTrace(_T("Driver Open success\n"));
    }

    return hDrv;

}


//
// Close Driver
//
void CloseDriver(HANDLE hDrv)
{
    CloseHandle(hDrv);

    DebugTrace(_T("Driver Close success\n"));
}



//--------------------------------------------------------------------------------
//  Device Enable/Disable Function
//--------------------------------------------------------------------------------



//
// Get Device Registory Property
//
PUCHAR GetDeviceRegistryProperty(
            HDEVINFO hDevInfo,
            PSP_DEVINFO_DATA pDevInfoData,
            DWORD property,
            DWORD *pBufferSize,
            DWORD *pError
            )
{
    UCHAR *pBuf = NULL;
    DWORD dwSize = 0;
    DWORD dwRet = ERROR_SUCCESS;
    BOOL bRet = FALSE;

    if(pBufferSize){
        *pBufferSize = 0;
    }

    while(!bRet){

        bRet = SetupDiGetDeviceRegistryProperty(
                                            hDevInfo,
                                            pDevInfoData,
                                            property,
                                            NULL,
                                            pBuf,
                                            dwSize,
                                            &dwSize
                                            );

        if(!bRet){
            dwRet = GetLastError();

            if(dwRet == ERROR_INSUFFICIENT_BUFFER){

                if(pBuf){
                    xhcfree(pBuf);
                }

                pBuf = (PBYTE)xhcmalloc(dwSize);


                if(pBuf){
                    if(pBufferSize){
                        *pBufferSize = dwSize;
                    }
                }
                else{
                    DebugTrace(_T("Memory allocate fail (%d)\n"),dwSize);
                    break;
                }
            }
            // error
            else{
                DebugTrace(_T("SetupDiGetDeviceRegistryProperty fail (%d)\n"),dwRet);
                break;
            }
        }
        else{
            dwRet = ERROR_SUCCESS;
        }
    }

    if(pError){
        *pError = dwRet;
    }

    return pBuf;

}


//
// Check Device Address
//
BOOL CheckDevAddress(
            HDEVINFO hDevInfo,
            PSP_DEVINFO_DATA pDevInfoData,
            USHORT devAddr,
            DWORD *pError
            )
{

    DWORD bus,address;
    DWORD dwSize;
    UCHAR *pBuf;

    DebugTrace(_T("CheckDevAddress = 0x%04X\n"),devAddr);

    // Get BusNumber
    pBuf = GetDeviceRegistryProperty(
                                    hDevInfo,
                                    pDevInfoData,
                                    SPDRP_BUSNUMBER,
                                    &dwSize,
                                    pError);

    if(!pBuf){
        return FALSE;
    }

    if(dwSize != sizeof(DWORD)){
        xhcfree(pBuf);
        *pError = ERROR_INVALID_DATA;
        DebugTrace(_T("SPDRP_BUSNUMBER size mismatch (%d) \n"),dwSize);
        return FALSE;
    }

    bus = *((DWORD*)pBuf);
    xhcfree(pBuf);

    DebugTrace(_T("Bus = 0x%08X\n"),bus);

    if(bus != GetBus(devAddr)){
        *pError = ERROR_INVALID_DATA;
        return FALSE;
    }


    // Get Address
    pBuf = GetDeviceRegistryProperty(
                                    hDevInfo,
                                    pDevInfoData,
                                    SPDRP_ADDRESS,
                                    &dwSize,
                                    pError);

    if(!pBuf){
        return FALSE;
    }

    if(dwSize != sizeof(DWORD)){
        xhcfree(pBuf);
        *pError = ERROR_INVALID_DATA;
        DebugTrace(_T("SPDRP_ADDRESS size mismatch (%d)\n"),dwSize);
        return FALSE;
    }

    address = *((DWORD*)pBuf);
    xhcfree(pBuf);

    DebugTrace(_T("Address = 0x%08X\n"),address);

    if(((address >> 16) & 0xFFFF) != GetDev(devAddr)){
        *pError = ERROR_INVALID_DATA;
        return FALSE;
    }

    if((address & 0xFFFF) != GetFunc(devAddr)){
        *pError = ERROR_INVALID_DATA;
        return FALSE;
    }

    return TRUE;

}


//
// Get Device Address
//
BOOL GetDevAddress(
            HDEVINFO hDevInfo,
            PSP_DEVINFO_DATA pDevInfoData,
            USHORT *pDevAddr,
            DWORD *pError
            )
{

    DWORD bus,address;
    DWORD dwSize;
    UCHAR *pBuf;

    DebugTrace(_T("GetDevAddress\n"));

    // Get BusNumber
    pBuf = GetDeviceRegistryProperty(
                                    hDevInfo,
                                    pDevInfoData,
                                    SPDRP_BUSNUMBER,
                                    &dwSize,
                                    pError);

    if(!pBuf){
        return FALSE;
    }

    if(dwSize != sizeof(DWORD)){
        xhcfree(pBuf);
        *pError = ERROR_INVALID_DATA;
        DebugTrace(_T("SPDRP_BUSNUMBER size mismatch (%d) \n"),dwSize);
        return FALSE;
    }

    bus = *((DWORD*)pBuf);
    xhcfree(pBuf);

    DebugTrace(_T("Bus = 0x%08X\n"),bus);

    // Get Address
    pBuf = GetDeviceRegistryProperty(
                                    hDevInfo,
                                    pDevInfoData,
                                    SPDRP_ADDRESS,
                                    &dwSize,
                                    pError);

    if(!pBuf){
        return FALSE;
    }

    if(dwSize != sizeof(DWORD)){
        xhcfree(pBuf);
        *pError = ERROR_INVALID_DATA;
        DebugTrace(_T("SPDRP_ADDRESS size mismatch (%d)\n"),dwSize);
        return FALSE;
    }

    address = *((DWORD*)pBuf);
    xhcfree(pBuf);

    DebugTrace(_T("Address = 0x%08X\n"),address);

    *pDevAddr = (USHORT)MakeDevAddr(bus,((address >> 16) & 0xFFFF),(address & 0xFFFF));

    DebugTrace(_T("DevAddr = 0x%08X\n"),*pDevAddr);


    return TRUE;

}


//
// Initilize List
//
void InitDisableList(void)
{
    InitializeListHead(&g_DisableList);
    g_DisableNum = 0;
}

//
// Add Disable Info
//
BOOL AddDisableDevice(USHORT devAddr)
{
    PDISABLE_DEVICE pDev;

    pDev = (PDISABLE_DEVICE)xhcmalloc(sizeof(DISABLE_DEVICE));

    if(!pDev){
        return FALSE;
    }

    pDev->DevAddr = devAddr;

    InsertTailList(&g_DisableList,&pDev->link);
    g_DisableNum++;

    return TRUE;
}

//
// Get Disable Info
//
PDISABLE_DEVICE GetDisableDevice(USHORT devAddr)
{
    PDISABLE_DEVICE pDev;
    PLIST_ENTRY pEntry;

    pEntry = g_DisableList.Flink;

    while(pEntry != &g_DisableList){
        pDev = CONTAINING_RECORD(pEntry,DISABLE_DEVICE,link);

        if(pDev->DevAddr == devAddr){
            return pDev;
        }

        pEntry = pEntry->Flink;
    }

    return NULL;
}


//
// Remove Disable Info
//
BOOL RemoveDisableDeviceEx(PDISABLE_DEVICE pDev)
{
    RemoveEntryList(&pDev->link);
    g_DisableNum--;

    xhcfree(pDev);

    return TRUE;
}


BOOL RemoveDisableDevice(USHORT devAddr)
{
    PDISABLE_DEVICE pDev;

    pDev = GetDisableDevice(devAddr);

    if(pDev){
        RemoveDisableDeviceEx(pDev);
        return TRUE;
    }

    return FALSE;

}




//
//
//
DEVINST CheckChild(DEVINST dnDevInst, UINT nChildLevel)
{
    DEVINST child = NULL;
    DEVINST current;
    DEVINST next;
    CONFIGRET cfgret;

    cfgret = CM_Get_Child(&current, dnDevInst, 0);

    if(cfgret != CR_SUCCESS){
        return NULL;
    }

    if(nChildLevel == 1){
        return current;
    }

    while(current){
        child = CheckChild(current,(nChildLevel-1));

        if(child){
            break;
        }

        cfgret = CM_Get_Sibling(&next, current, 0);

        if(cfgret != CR_SUCCESS){
            break;
        }

        current = next;

    }

    return child;
}


//
// Disable Device
//
int DisableDevice(USHORT devAddr, UINT nChildLevel, DWORD *pError)
{
    HDEVINFO hDevInfo = NULL;
    SP_DEVINFO_DATA devInfoData;
    SP_PROPCHANGE_PARAMS pcp;
    DWORD dwFlag = DIGCF_ALLCLASSES | DIGCF_PRESENT;
    DWORD dwErr = ERROR_SUCCESS;
    CONFIGRET cfgret;
    ULONG devStatus;
    ULONG problemNumber;
    BOOL bFound = FALSE;

    CString Enum;
    int count(0);
    int ret = XHCSTS_SUCCESS;

    DebugTrace(_T(__FUNCTION__) _T(" ++\n"));
    DebugTrace(_T("Target device address = 0x%04X\n"),devAddr);

    Enum = _T("PCI");

    hDevInfo = SetupDiGetClassDevs(NULL,Enum,NULL,dwFlag);
//  hDevInfo = SetupDiGetClassDevs(NULL,NULL,NULL,dwFlag);

    if(!hDevInfo){
        dwErr = GetLastError();
        goto Exit_DisableDevice;
    }

    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    while(1){
        if(!SetupDiEnumDeviceInfo(hDevInfo,count++,&devInfoData)){
            dwErr = GetLastError();
            break;
        }

        // Check device address
        if(!CheckDevAddress(hDevInfo, &devInfoData, devAddr, &dwErr)){
            if(dwErr == ERROR_INVALID_DATA){
                continue;
            }

            break;
        }

        DebugTrace(_T("Target Device Found!\n"));
        bFound = TRUE;

        if(nChildLevel){
            if(CheckChild(devInfoData.DevInst,nChildLevel) != NULL){
                DebugTrace(_T("Target Device Children Found!\n"));
                dwErr = ERROR_SUCCESS;
                ret = XHCSTS_CHDCHK_ERROR;
                break;
            }
        }

        cfgret = CM_Get_DevNode_Status(&devStatus,&problemNumber,devInfoData.DevInst,0);

        if(cfgret == CR_SUCCESS){

            pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            pcp.HwProfile = 0;

            if(devStatus & DN_STARTED){
                pcp.StateChange = DICS_DISABLE;
            }
            else if(problemNumber & CM_PROB_FAILED_START){
                pcp.StateChange = DICS_DISABLE;
            }
            else{
                dwErr = ERROR_SUCCESS;
                DebugTrace(_T("Device not started!\n"),dwErr);
                break;
            }


            pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;

            if(!SetupDiSetClassInstallParams(hDevInfo,&devInfoData,&pcp.ClassInstallHeader,sizeof(pcp))){
                // Error
                dwErr = GetLastError();
                DebugTrace(_T("SetupDiSetClassInstallParams(2) Faild! (%d)\n"),dwErr);
                break;
            }

            if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&devInfoData)) {
                // Error
                dwErr = GetLastError();
                DebugTrace(_T("SetupDiCallClassInstaller(2) Faild! (%d)\n"),dwErr);
                break;
            }

            DebugTrace(_T("Device disable success!\n"));

            if(!AddDisableDevice(devAddr)){
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                DebugTrace(_T("AddDisableDevice Faild! (%d)\n"),dwErr);
                break;
            }

            dwErr = ERROR_SUCCESS;
            break;

        }
        else{
            dwErr = cfgret;
            DebugTrace(_T("CM_Get_DevNode_Status Faild! (%d)\n"),cfgret);
            break;
        }
    }

Exit_DisableDevice:

    if(hDevInfo){
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    if(pError){
        *pError = dwErr;
    }

    if((ret == XHCSTS_SUCCESS) &&
       (dwErr != ERROR_SUCCESS)){
        ret = XHCSTS_DEVMGR_ERROR;
    }

    DebugTrace(_T(__FUNCTION__) _T(" -- (win32=%d - xhc201=%d)\n"),dwErr, ret);

    return ret;
}



//
// Enable Device
//
int EnableDevice(USHORT devAddr, DWORD *pError)
{
    HDEVINFO hDevInfo = NULL;
    SP_DEVINFO_DATA devInfoData;
    SP_PROPCHANGE_PARAMS pcp;
    DWORD dwFlag = DIGCF_ALLCLASSES | DIGCF_PRESENT;
    DWORD dwErr = ERROR_SUCCESS;
    CONFIGRET cfgret;
    ULONG devStatus;
    ULONG problemNumber;
    PDISABLE_DEVICE pDev;
    BOOL bFound = FALSE;

    CString Enum;
    int count(0);
    int ret = XHCSTS_SUCCESS;

    DebugTrace(_T(__FUNCTION__) _T(" ++\n"));
    DebugTrace(_T("Target device address = 0x%04X\n"),devAddr);


    pDev = GetDisableDevice(devAddr);

    if(!pDev){
        dwErr = ERROR_INVALID_HANDLE;
        goto Exit_EnableDevice;
    }

    Enum = _T("PCI");

    hDevInfo = SetupDiGetClassDevs(NULL,Enum,NULL,dwFlag);
//  hDevInfo = SetupDiGetClassDevs(NULL,NULL,NULL,dwFlag);

    if(!hDevInfo){
        dwErr = GetLastError();
        goto Exit_EnableDevice;
    }

    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    while(1){
        if(!SetupDiEnumDeviceInfo(hDevInfo,count++,&devInfoData)){
            dwErr = GetLastError();
            break;
        }

        // Check device address
        if(!CheckDevAddress(hDevInfo, &devInfoData, devAddr, &dwErr)){
            if(dwErr == ERROR_INVALID_DATA){
                continue;
            }

            break;
        }

        DebugTrace(_T("Target Device Found!\n"));

        bFound = TRUE;

        cfgret = CM_Get_DevNode_Status(&devStatus,&problemNumber,devInfoData.DevInst,0);

        if(cfgret == CR_SUCCESS){

            pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            pcp.HwProfile = 0;

            if(problemNumber & CM_PROB_DISABLED){
                pcp.Scope = DICS_FLAG_GLOBAL;
                pcp.StateChange = DICS_ENABLE;

                if(!SetupDiSetClassInstallParams(hDevInfo,&devInfoData,&pcp.ClassInstallHeader,sizeof(pcp))){
                    // Error
                    dwErr = GetLastError();
                    DebugTrace(_T("SetupDiSetClassInstallParams Faild! (%d)\n"),dwErr);
                    break;
                }

                if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&devInfoData)) {
                    // Error
                    dwErr = GetLastError();
                    DebugTrace(_T("SetupDiCallClassInstaller Faild! (%d)\n"),dwErr);

                    if(dwErr != ERROR_DI_DO_DEFAULT){
                        break;
                    }

                    break;
                }
            }
            else{

                dwErr = ERROR_SUCCESS;
                DebugTrace(_T("Device not disabled!\n"),dwErr);
                break;
            }


            pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;

            if(!SetupDiSetClassInstallParams(hDevInfo,&devInfoData,&pcp.ClassInstallHeader,sizeof(pcp))){
                // Error
                dwErr = GetLastError();
                DebugTrace(_T("SetupDiSetClassInstallParams(2) Faild! (%d)\n"),dwErr);
                break;
            }

            if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&devInfoData)) {
                // Error
                dwErr = GetLastError();
                DebugTrace(_T("SetupDiCallClassInstaller(2) Faild! (%d)\n"),dwErr);
                break;
            }

            DebugTrace(_T("Device enable success!\n"));

            RemoveDisableDeviceEx(pDev);

            dwErr = ERROR_SUCCESS;

            break;

        }
        else{
            dwErr = cfgret;
            DebugTrace(_T("CM_Get_DevNode_Status Faild! (%d)\n"),cfgret);
            break;
        }
    }

Exit_EnableDevice:

    if(hDevInfo){
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    if(pError){
        *pError = dwErr;
    }

    if((ret == XHCSTS_SUCCESS) &&
       (dwErr != ERROR_SUCCESS)){
        ret = XHCSTS_DEVMGR_ERROR;
    }

    DebugTrace(_T(__FUNCTION__) _T(" -- (win32=%d - xhc201=%d)\n"),dwErr, ret);


    return ret;
}



//
// Enable All Device
//
void EnableAllDevice(void)
{
    HDEVINFO hDevInfo = NULL;
    SP_DEVINFO_DATA devInfoData;
    SP_PROPCHANGE_PARAMS pcp;
    DWORD dwFlag = DIGCF_ALLCLASSES | DIGCF_PRESENT;
    DWORD dwErr = ERROR_SUCCESS;
    USHORT devAddr;
    CONFIGRET cfgret;
    ULONG devStatus;
    ULONG problemNumber;
    PDISABLE_DEVICE pDev;
    PLIST_ENTRY pEntry;

    CString Enum;
    int count(0);

    DebugTrace(_T(__FUNCTION__) _T(" ++\n"));

    Enum = _T("PCI");

    hDevInfo = SetupDiGetClassDevs(NULL,Enum,NULL,dwFlag);
//  hDevInfo = SetupDiGetClassDevs(NULL,NULL,NULL,dwFlag);

    if(!hDevInfo){
        dwErr = GetLastError();
        goto Exit_EnableAllDevice;
    }

    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    while(g_DisableNum){
        if(!SetupDiEnumDeviceInfo(hDevInfo,count++,&devInfoData)){
            dwErr = GetLastError();
            break;
        }

        // Check device address
        if(!GetDevAddress(hDevInfo, &devInfoData, &devAddr, &dwErr)){
            continue;
        }

        pDev = GetDisableDevice(devAddr);

        if(!pDev){
            continue;
        }

        DebugTrace(_T("Target Device(0x%04X) Found!\n"),devAddr);

        cfgret = CM_Get_DevNode_Status(&devStatus,&problemNumber,devInfoData.DevInst,0);

        if(cfgret == CR_SUCCESS){

            pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            pcp.HwProfile = 0;

            if(problemNumber & CM_PROB_DISABLED){
                pcp.Scope = DICS_FLAG_GLOBAL;
                pcp.StateChange = DICS_ENABLE;

                if(!SetupDiSetClassInstallParams(hDevInfo,&devInfoData,&pcp.ClassInstallHeader,sizeof(pcp))){
                    // Error
                    dwErr = GetLastError();
                    DebugTrace(_T("SetupDiSetClassInstallParams Faild! (%d)\n"),dwErr);
                    continue;
                }

                if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&devInfoData)) {
                    // Error
                    dwErr = GetLastError();
                    DebugTrace(_T("SetupDiCallClassInstaller Faild! (%d)\n"),dwErr);
                    continue;
                }
            }
            else{

                dwErr = ERROR_SUCCESS;
                DebugTrace(_T("Device not disabled!\n"),dwErr);
                continue;
            }


            pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;

            if(!SetupDiSetClassInstallParams(hDevInfo,&devInfoData,&pcp.ClassInstallHeader,sizeof(pcp))){
                // Error
                dwErr = GetLastError();
                DebugTrace(_T("SetupDiSetClassInstallParams(2) Faild! (%d)\n"),dwErr);
                continue;
            }

            if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&devInfoData)) {
                // Error
                dwErr = GetLastError();
                DebugTrace(_T("SetupDiCallClassInstaller(2) Faild! (%d)\n"),dwErr);
                continue;
            }

            DebugTrace(_T("Device enable success!\n"));

            RemoveDisableDeviceEx(pDev);

            dwErr = ERROR_SUCCESS;
        }
        else{
            dwErr = cfgret;
            DebugTrace(_T("CM_Get_DevNode_Status Faild! (%d)\n"),cfgret);
            continue;
        }
    }

Exit_EnableAllDevice:

    if(hDevInfo){
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    while(!IsListEmpty(&g_DisableList)){
        pEntry = RemoveHeadList(&g_DisableList);

        pDev = CONTAINING_RECORD(pEntry,DISABLE_DEVICE,link);

        DebugTrace(_T("Enable fail device = 0x%04X\n"),pDev->DevAddr);

        xhcfree(pDev);

        g_DisableNum--;
    }


    DebugTrace(_T(__FUNCTION__) _T(" -- (%d)\n"),dwErr);

}


