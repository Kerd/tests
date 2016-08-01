#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>

#pragma comment(lib, "SetupAPI.lib")

static void win32perror(const char *prefix)
{
    LPVOID buf;
    DWORD dw = GetLastError();
    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL, dw,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR) &buf,
                      0,
                      NULL))
    {
        fprintf(stderr, "%s: Error [0x%08X] %s", prefix, dw, buf);
        LocalFree(buf);
    } else
        fprintf(stderr, "%s: Error [0x%08X] unknown error", prefix, dw);
}

/*
DWORD  GetSystemErrorStr(DWORD lasterr, char* buf, DWORD len)
{
    return FormatMessage(
	FORMAT_MESSAGE_FROM_SYSTEM |
	FORMAT_MESSAGE_IGNORE_INSERTS,
	NULL,
        lasterr,
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	(LPTSTR) buf, len, NULL);
}

void LogSystemError(const char* prefix) 
{ 
    char    buf[1024];
    DWORD   dw = GetLastError();
    buf[0] = '\0';
    GetSystemErrorStr(dw, buf, sizeof(buf));
    log_printf(elog_err, "%s: Error [0x%08X] %s", prefix, dw, buf);
}
*/

LPTSTR get_str_property(HDEVINFO hDevInfo, SP_DEVINFO_DATA* DeviceInfoData, DWORD prop)
{
        DWORD DataT;
        LPTSTR buffer = NULL;
        DWORD buffersize = 0;
        
        //
        // Call function with null to begin with, 
        // then use the returned buffer size (doubled)
        // to Alloc the buffer. Keep calling until
        // success or an unknown failure.
        //
        //  Double the returned buffersize to correct
        //  for underlying legacy CM functions that 
        //  return an incorrect buffersize value on 
        //  DBCS/MBCS systems.
        // 
        while (!SetupDiGetDeviceRegistryProperty(hDevInfo, DeviceInfoData,
            prop, &DataT,
            (PBYTE)buffer,
            buffersize,
            &buffersize))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                // Change the buffer size.
                if (buffer) LocalFree(buffer);
                // Double the size to avoid problems on 
                // W2k MBCS systems per KB 888609. 
                buffer = LocalAlloc(LPTR,buffersize * 2);

		if (!buffer)
		    return NULL;
            }
            else
            {
                // Insert error handling here.
		win32perror("SetupDiGetDeviceRegistryProperty()");
                if (buffer) LocalFree(buffer);
		return NULL;
            }
        }

	return buffer;
}

void show_property(HDEVINFO hDevInfo, SP_DEVINFO_DATA* DeviceInfoData, DWORD prop, const char* name)
{
    LPTSTR buffer = get_str_property(hDevInfo, DeviceInfoData, prop);
    printf("%s: [%s]\n", name, buffer ? buffer : "N/A");
    if (buffer) LocalFree(buffer);
}

int main( int argc, char *argv[ ], char *envp[ ] )
{
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD i;

/*
    hDevInfo = SetupDiGetClassDevs(NULL,
        REGSTR_KEY_PCIENUM, // Enumerator
        0,
        DIGCF_PRESENT | DIGCF_ALLCLASSES );
*/

    // Create a HDEVINFO with all present devices.
    hDevInfo = SetupDiGetClassDevs(
	&GUID_DEVINTERFACE_TAPE,
        0, // Enumerator
        0,
	DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
//        DIGCF_PRESENT | DIGCF_ALLCLASSES );
    
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        // Insert error handling here.
        return 1;
    }
    
    // Enumerate through all devices in Set.
    
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i=0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
    {
	show_property(hDevInfo, &DeviceInfoData, SPDRP_DEVTYPE, "Type");
	show_property(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, "Description");
	show_property(hDevInfo, &DeviceInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, "PhysDevName");

    }

    
    
    if ( GetLastError()!=NO_ERROR &&
         GetLastError()!=ERROR_NO_MORE_ITEMS )
    {
        // Insert error handling here.
        return 1;
    }
    
    //  Cleanup
    SetupDiDestroyDeviceInfoList(hDevInfo);
    
    return 0;
}