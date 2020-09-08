#pragma comment (lib, "setupapi.lib")
#include <iostream>
#include <windows.h>
#include <SetupAPI.h>
#include <setupapi.h>
#include <regex>
#include <iomanip>

using namespace std;

string getId(TCHAR* deviceInfo, string regexPattern) {
regex pattern(regexPattern); // regex pattern;
smatch match; // matches
string device = deviceInfo; // deviceInfo TCHAR* to string
regex_search(device, match, pattern);
if (match.length() > 1) {
	return match[1];
}
return "No ID found";
}


int main() {
	setlocale(LC_ALL, "Russian");
	HDEVINFO handle;

	handle = SetupDiGetClassDevs(NULL, "PCI", NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
	if (handle == INVALID_HANDLE_VALUE) {
		cout << "Error";
		return 1;
	}

	SP_DEVINFO_DATA info;
	info.cbSize = sizeof(SP_DEVINFO_DATA);
	PBYTE buffer = nullptr;
	DWORD requiredSize;
	cout << setw(90) << left << "Name";
	cout << setw(15) << left << "DeviceID";
	cout << setw(15) << left << "VendorID" << endl;
	while (SetupDiEnumDeviceInfo(handle, 0, &info)) {	
		
		SetupDiGetDeviceRegistryProperty(handle, &info, SPDRP_DEVICEDESC, NULL, NULL, 0, &requiredSize);
		buffer = new BYTE[requiredSize];
		SetupDiGetDeviceRegistryProperty(handle, &info, SPDRP_DEVICEDESC, NULL, buffer, requiredSize, NULL);
		cout << setw(90) << left << buffer;

		SetupDiGetDeviceRegistryProperty(handle, &info, SPDRP_HARDWAREID, NULL, NULL, 0, &requiredSize);
		buffer = new BYTE[requiredSize];
		SetupDiGetDeviceRegistryProperty(handle, &info, SPDRP_HARDWAREID, NULL, buffer, requiredSize, NULL);
		string vendorId = getId((TCHAR*)buffer, "VEN_(\\w+)");
		string deviceId = getId((TCHAR*)buffer, "DEV_(\\w+)");

		cout << setw(15) << left << vendorId;
		cout << setw(15) << left << deviceId << endl;

		SetupDiDeleteDeviceInfo(handle, &info);

		
	}
		
	getchar();
	return 0;
	
}
