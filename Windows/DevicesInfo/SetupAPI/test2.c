/*

cs-win-001.cpp

Jason Tost
Wavelength Electronics Inc.
2007-05-25

Dependencies:
   setupapi.lib  - Installed with Driver Developer Kit
   ole32.lib     - Installed with Visual Studio

Reference implementation of WEIUSB device enumeration using the Win32 APIs in
Visual C++ 6.  Implemented as a console app that simply prints device instances
to the console.

*/

#include <windows.h>  // Required for data types
#include <ole2.h>     // Required for CLSIDFromString()
#include <setupapi.h> // Device setup APIs
#include <stdio.h>    // For console output

#pragma comment(lib, "SetupAPI.lib")
#pragma comment(lib, "ole32.lib")


// Define the WEIUSB device class string.  This class indicates in this
// case the device class associated with the WEIUSB device under Windows.
#define CLSID_STR_WEIUSB (L"{3A8DB48F-E27D-49C9-A7A9-90CDE2DBFC0E}")

int main(int argc, char* argv[])
{
	GUID     DevClass;    // CLSID holder for the WEIUSB device class
	HDEVINFO hDevInfoSet; // Handle to device information set
	DWORD    dwIndex;     // Index of current device info set record
	DWORD    dwRequired;  // Required buffer length
	DWORD    dwError;     // Error value from GetLastError()
	BOOL     bResult;     // Boolean return result value

	SP_DEVICE_INTERFACE_DATA         DevNode;
	PSP_DEVICE_INTERFACE_DETAIL_DATA DevDetails;

	// Convert the registry-formatted CLSID for the WEIUSB device
	// class to a GUID structure.
//	CLSIDFromString(CLSID_STR_WEIUSB, &DevClass);
	DevClass = GUID_DEVINTERFACE_TAPE;
//	DevClass = GUID_DEVINTERFACE_CDROM;

	// Generate the device information set.  This is the actual
	// enumeration process.  We are specifying the class GUID of
	// the device type we want to enumerate, in this case only
	// WEIUSB devices.  The second argument would allow us to
	// constrain the enumeration to those devices attached to a
	// specified enumerator, or bus.  We could, for example,
	// enumerate only those devices attached via USB.
	hDevInfoSet = SetupDiGetClassDevs(
		&DevClass, // Only get WEIUSB devices
		NULL,      // Not specific to any bus
		NULL,      // Not associated with any window
		DIGCF_DEVICEINTERFACE + DIGCF_PRESENT);

	// Make sure enumeration completed without errors.
	if (hDevInfoSet == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr,
			"Unable to create device information set (Error 0x%08X)\n",
			GetLastError());
		return 1;
	}
	printf("Successfully created device information set.\n");

	// Iterate through the device info set.
	for (dwIndex = 0; ; dwIndex++)
	{
		// Retrieve the data from the next node index within the
		// device information set.
		DevNode.cbSize = sizeof(DevNode);
		bResult = SetupDiEnumDeviceInterfaces(
			hDevInfoSet,   // Handle to device info set
			NULL,          // Do not apply advanced filtering
			&DevClass,     // Class of device to retrieve
			dwIndex,       // List index of requested record
			&DevNode);     // Pointer to structure to receive data

		// If the previous call failed, do not continue trying
		// to enumerate devices.
		if (!bResult)
		{
			dwError = GetLastError();
			if (dwError != ERROR_NO_MORE_ITEMS)
			{
				fprintf(stderr,
					"Error enumerating devices (0x%08X).\n",
					dwError);
			}
			break;
		}

		// The device information data represents some device
		// flags and the class GUID associated with the device
		// instance.  The device details must be retrieved to
		// get the device path that can be used to open the
		// device.  This is a two-step process.  First the API
		// must be called with the pointer to a buffer set to
		// NULL, and the size of the buffer specified as zero.
		// This will cause the API to fail, but the API will
		// provide a value in the dwRequired argument indicating
		// how much memory is needed to store the file path.
		// The memory can then be allocated and the API called a
		// second time, specifying the pointer to the buffer and
		// the buffer size, to receive the actual device path.
		SetupDiGetDeviceInterfaceDetail(
			hDevInfoSet,  // Handle to device information set
			&DevNode,     // Specify a pointer to the current node
			NULL,         // Pointer to structure to receive path data
			0,            // Currently no space is allocated
			&dwRequired,  // Pointer to var to receive required buffer size
			NULL);        // Pointer to var to receive additional device info

		// Allocate memory required.
		DevDetails = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(dwRequired);
		if (DevDetails == NULL)
		{
			fprintf(stderr,
				"Unable to allocate memory for buffer.  Stopping.\n");
			break;
		}

		// Initialize the structure before using it.
		memset(DevDetails, 0, dwRequired);
		DevDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Call the API a second time to retrieve the actual
		// device path string.
		bResult = SetupDiGetDeviceInterfaceDetail(
			hDevInfoSet,  // Handle to device information set
			&DevNode,     // Pointer to current node in devinfo set
			DevDetails,   // Pointer to buffer to receive device path
			dwRequired,   // Length of user-allocated buffer
			&dwRequired,  // Pointer to arg to receive required buffer length
			NULL);        // Not interested in additional data

		if (!bResult)
		{
			// Some error occurred retrieve the device path.
			fprintf(stderr,
				"ERROR: Unable to retrieve device path (0x%08X).\n",
				GetLastError());
		}
		else
		{
			// Successfully retrieved the device path.  Go ahead
			// and print it to the console.
			printf("   %s\n", DevDetails->DevicePath);
		}

		// Deallocate memory used
		free(DevDetails);

	} // for (dwIndex = 0;; dwIndex++)

	// Print output notification for user.
	printf("Enumeration listing complete.  %d device(s) detected.\n", dwIndex);

	// Deallocate resources used by Windows for enumeration.
	SetupDiDestroyDeviceInfoList(hDevInfoSet);

	return 0;
}
