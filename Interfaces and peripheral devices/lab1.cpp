#pragma comment (lib, "setupapi.lib")
#include <iostream>
#include <windows.h>
#include <SetupAPI.h>
#include <setupapi.h>
#include <regex>
#include <iomanip>

using namespace std;

string getId(TCHAR* deviceInfo, string regexPattern) {
	string result;
	regex pattern(regexPattern); // regex pattern;
	smatch match; // matches
	string device = deviceInfo; // deviceInfo TCHAR* to string
	regex_search(device, match, pattern);

	if (match.length() > 1) {
		result = match[1];
	}
	else {
		result = "None";
	}
	return result;
}


int main() {
	setlocale(LC_ALL, "Russian");
	HDEVINFO handle;

	handle = SetupDiGetClassDevs(NULL,		// GUID (Globally Unique Identifier) of device or interface
		"PCI",								//ID of a Plug and Play ((GUID) or symbolic name)
		NULL,								//handle to the top-level window for a user interface associated with installing
		DIGCF_ALLCLASSES | DIGCF_PRESENT);	//list of all installed devices, only devices that are currently present in a system; return handle to device info set
	if (handle == INVALID_HANDLE_VALUE) {
		cout << GetLastError();
		return 1;
	}

	SP_DEVINFO_DATA info;					//device information element in a device information set.
	info.cbSize = sizeof(SP_DEVINFO_DATA);
	PBYTE buffer = nullptr;
	DWORD requiredSize;

	cout << setw(90) << left << "Name";
	cout << setw(15) << left << "DeviceID";
	cout << setw(15) << left << "VendorID" << endl;

	while (SetupDiEnumDeviceInfo(handle, 0, &info)) {		// handle of set, index, structure 
		SetupDiGetDeviceRegistryProperty(handle, &info, SPDRP_DEVICEDESC, NULL, NULL, 0, &requiredSize);	//description of device, pointer receives the data type, p buffer, size buffer
		buffer = new BYTE[requiredSize];
		SetupDiGetDeviceRegistryProperty(handle, &info, SPDRP_DEVICEDESC, NULL, buffer, requiredSize, NULL);
		cout << setw(90) << left << buffer;

		SetupDiGetDeviceRegistryProperty(handle, &info, SPDRP_HARDWAREID, NULL, NULL, 0, &requiredSize);
		buffer = new BYTE[requiredSize];
		SetupDiGetDeviceRegistryProperty(handle, &info, SPDRP_HARDWAREID, NULL, buffer, requiredSize, NULL);
		
		string vendorId = getId((TCHAR*)buffer, "VEN_(\\w+)");
		string deviceId = getId((TCHAR*)buffer, "DEV_(\\w+)");
		cout << setw(15) << left << deviceId;
		cout << setw(15) << left << vendorId << endl;

		SetupDiDeleteDeviceInfo(handle, &info);

		
	}
		
	getchar();
	return 0;
	
}
