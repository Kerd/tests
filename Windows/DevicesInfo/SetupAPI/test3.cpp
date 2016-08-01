// USB_Drive_Property.cpp : Defines the entry point for the console application.
//
#define INITGUID
 
#include <windows.h>
#include <SetupApi.h> // Included from Windows SDKs
#include <stdio.h>
 
#pragma comment(lib, "SetupAPI.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
 
// The one and only application object
 
using namespace std;
 
//DEFINE_GUID(GUID_DEVINTERFACE_VOLUME, 0x53f5630dL, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);

//DEFINE_GUID(GUID_StorageClasses, 0x722E224CL, 0x223C, 0x722E, 0x28, 0x0F0, 0x2E, 0x72, 0x1C, 0x22, 0x2E, 0x72);

int main(int argc, TCHAR* argv[], TCHAR* envp[])
{
    int nRetCode = 0;
   
    HDEVINFO hDevHandle;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    DWORD required = 0;
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
 
    int nBufferSize = 0;
    
    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
 
    DWORD MemberIndex = 0;
    BOOL  Result;
 
    const GUID* devCls = &GUID_DEVINTERFACE_STORAGEPORT;

    hDevHandle = SetupDiGetClassDevs(devCls, NULL, NULL, 
	DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
 
    if(hDevHandle == INVALID_HANDLE_VALUE)
    {
       return 1;
    }

    BOOL bStart = false;
    TCHAR *buffer = NULL;
    PSP_DEVICE_INTERFACE_DETAIL_DATA devicedetailData;
    do {
	Result = SetupDiEnumDeviceInfo(hDevHandle, MemberIndex, &devInfoData);
 
	if (Result) {
	    Result = SetupDiEnumDeviceInterfaces(hDevHandle, 0, 
		devCls, 
		MemberIndex, &deviceInterfaceData);
       	}
        if ( !Result )
        {
	    SetupDiDestroyDeviceInfoList(hDevHandle);
	    delete []buffer;
	    buffer = NULL;
	    return 1;
	}
	MemberIndex++;
 
	BOOL detailResult = FALSE;
    
	if(!bStart)
	{
          // As per MSDN, Get the required buffer size. Call SetupDiGetDeviceInterfaceDetail with a 
          // NULL DeviceInterfaceDetailData pointer, a DeviceInterfaceDetailDataSize of zero, 
          // and a valid RequiredSize variable. In response to such a call, this function returns 
          // the required buffer size at RequiredSize and fails with GetLastError returning 
          // ERROR_INSUFFICIENT_BUFFER. 
          // Allocate an appropriately sized buffer and call the function again to get the interface details. 
 
	    SetupDiGetDeviceInterfaceDetail(hDevHandle, &deviceInterfaceData, NULL, 0, &required, NULL);
 
            buffer = new char[required];
            devicedetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) buffer;
            devicedetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
            nBufferSize = required;
            bStart = true;
	}
 
	detailResult = SetupDiGetDeviceInterfaceDetail(hDevHandle, &deviceInterfaceData, 
		devicedetailData, nBufferSize , &required, NULL);
 
	printf("Device Path = %s\n\n", devicedetailData->DevicePath);
 
        if(detailResult)
        {
           char szDescription[MAX_PATH];
           memset(szDescription, 0, MAX_PATH);
        
           SetupDiGetClassDescription(&devInfoData.ClassGuid, szDescription, MAX_PATH, &required);
           printf("Class Description = %s\n\n", szDescription);
        
           memset(szDescription, 0, MAX_PATH);
           SetupDiGetDeviceInstanceId(hDevHandle, &devInfoData, szDescription, MAX_PATH, 0);
           printf("Device Instance Id = %s\n\n\n", szDescription);
        }
        
        if(!detailResult)
        {
           continue;
        }
    }while(Result);
 
    return nRetCode;
}