#include <iostream>
#include <iomanip>
#include <Windows.h>
#include <ntddscsi.h>
#include <string>

using namespace std;

#define bThousand 1024
#define BYTE_SIZE 8

const char* busType[] = { "UNKNOWN", "SCSI", "ATAPI", "ATA", "ONE_TREE_NINE_FOUR",
"SSA", "FIBRE", "USB", "RAID", "ISCSI", "SAS", "SATA", "SD", "MMC" };


void getDeviceInfo(HANDLE diskHandle, STORAGE_PROPERTY_QUERY storageProtertyQuery);
void getSupportModes(HANDLE diskHandle);
void GetSize(HANDLE dhandle, int counter);
bool skip;

int main() {

	int counter = 0;

	STORAGE_PROPERTY_QUERY storagePropertyQuery; 
	storagePropertyQuery.QueryType = PropertyStandardQuery;  
	storagePropertyQuery.PropertyId = StorageDeviceProperty; 

	while (true) {
		skip = false;
		string path = "\\\\.\\PhysicalDrive" + to_string(counter);

		HANDLE diskHandle = CreateFile(path.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,					
			OPEN_EXISTING,			
			NULL,					
			NULL);					

		if (diskHandle == INVALID_HANDLE_VALUE) {
			if (GetLastError() == 5)
			{
				cout << "Required access rights - administrator." << endl;
			}
			else if (GetLastError() == 2) {
				cout << "No disks more." << endl;
			}
			system("pause");
			return -1;
		}

		getDeviceInfo(diskHandle, storagePropertyQuery);
		if (!skip) {
			getSupportModes(diskHandle);
			GetSize(diskHandle, counter);

			cout << "----------------------------------------" << endl;
		}

		CloseHandle(diskHandle);
		counter++;
	}

	system("pause");
	return 0;
}

void getDeviceInfo(HANDLE diskHandle, STORAGE_PROPERTY_QUERY storageProtertyQuery) {
	STORAGE_DEVICE_DESCRIPTOR* deviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)calloc(bThousand,1); 
	deviceDescriptor->Size = bThousand;

	if (!DeviceIoControl(diskHandle, IOCTL_STORAGE_QUERY_PROPERTY, &storageProtertyQuery, sizeof(storageProtertyQuery), deviceDescriptor, bThousand, NULL, NULL)) { 
		printf("%d", GetLastError());
		CloseHandle(diskHandle);
		exit(-1);
	}
	if (busType[deviceDescriptor->BusType] == (char*)"USB") {
		skip = true;
		return;
	}
	cout << "Type of disk: " << "HDD" << endl;
	cout << "Product ID(Model and name): " << (char*)(deviceDescriptor) + deviceDescriptor->ProductIdOffset << endl;
	cout << "Version: " << (char*)(deviceDescriptor) + deviceDescriptor->ProductRevisionOffset << endl;
	cout << "Bus type: " << busType[deviceDescriptor->BusType] << endl;
	cout << "Serial number: " << (char*)(deviceDescriptor) + deviceDescriptor->SerialNumberOffset << endl;
	
}

void getSupportModes(HANDLE diskHandle) {

	UCHAR identifyDataBuffer[512 + sizeof(ATA_PASS_THROUGH_EX)] = { 0 };

	ATA_PASS_THROUGH_EX& PTE = *(ATA_PASS_THROUGH_EX*)identifyDataBuffer;
	PTE.Length = sizeof(PTE);		
	PTE.AtaFlags = ATA_FLAGS_DATA_IN;	
	PTE.DataTransferLength = 512; 
	PTE.TimeOutValue = 10;															
	PTE.DataBufferOffset = sizeof(ATA_PASS_THROUGH_EX);			
						

	IDEREGS* ideRegs = (IDEREGS*)PTE.CurrentTaskFile;
	ideRegs->bCommandReg = 0xEC;		

	if (!DeviceIoControl(diskHandle,
		IOCTL_ATA_PASS_THROUGH,								
		&PTE, sizeof(identifyDataBuffer), &PTE, sizeof(identifyDataBuffer), NULL, NULL)) {
		cout << GetLastError() << std::endl;
		return;
	}

	WORD* data = (WORD*)(identifyDataBuffer + sizeof(ATA_PASS_THROUGH_EX));	
	
	short ataSupportByte = data[80];
	int i = 2 * BYTE_SIZE;
	int bitArray[2 * BYTE_SIZE];

	while (i--) {
		bitArray[i] = ataSupportByte & 32768 ? 1 : 0;
		ataSupportByte = ataSupportByte << 1;
	}

	cout << "ATA Support:   ";
	for (int i = 8; i >= 4; i--) {
		if (bitArray[i] == 1) {
			cout << "ATA" << i;
			if (i != 4) {
				cout << ", ";
			}
		}
	}
	cout << endl;


	unsigned short dmaSupportedBytes = data[63];
	int i2 = 2 * BYTE_SIZE;

	while (i2--) {
		bitArray[i2] = dmaSupportedBytes & 32768 ? 1 : 0;
		dmaSupportedBytes = dmaSupportedBytes << 1;
	}

	cout << "DMA Support:   ";
	for (int i = 0; i < 3; i++) {
		if (bitArray[i] == 1) {
			cout << "DMA" << i;
			if (i != 2) cout << ", ";
		}
	}
	cout << endl;

	unsigned short pioSupportedBytes = data[63];
	int i3 = 2 * BYTE_SIZE;

	while (i3--) {
		bitArray[i3] = pioSupportedBytes & 32768 ? 1 : 0;
		pioSupportedBytes = pioSupportedBytes << 1;
	}

	cout << "PIO Support:   ";
	for (int i = 0; i < 2; i++) {
		if (bitArray[i] == 1) {
			cout << "PIO" << i + 3;
			if (i != 1) cout << ", ";
		}
	}
	cout << endl;
}

void GetSize(HANDLE dhandle, int counter)
{
	ULARGE_INTEGER all, free, occupied, allHDD, freeHDD, occupiedHDD;
	all.QuadPart = free.QuadPart = occupied.QuadPart = allHDD.QuadPart = freeHDD.QuadPart = occupiedHDD.QuadPart = 0;
	string path = "\\\\.\\";
	HANDLE logicDiskHandle = INVALID_HANDLE_VALUE;
	DISK_GEOMETRY diskGeometry = { 0 };
	STORAGE_DEVICE_NUMBER* deviceNumber;
	char* outBuffer = new char[BUFSIZ];
	memset(outBuffer, 0, BUFSIZ);
	int n;
	string bigPathForHandle;
	char symbolOfDisk;
	
	cout << "Memory: " << endl;

	if (!DeviceIoControl(dhandle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskGeometry, sizeof(DISK_GEOMETRY), NULL, NULL)){
		cout << "Error " << GetLastError() << endl;
	}
	long long driveSize = ((diskGeometry.Cylinders.QuadPart) * diskGeometry.TracksPerCylinder * diskGeometry.SectorsPerTrack * diskGeometry.BytesPerSector);


	DWORD logicalDrivesMask = GetLogicalDrives();
	for (int i = 0; i < 26; i++){

		n = ((logicalDrivesMask >> i) & 0x00000001);
		if (n == 1){

			symbolOfDisk = char(65 + i);
			bigPathForHandle = path + symbolOfDisk + ":";
			string shortPath;
			shortPath = symbolOfDisk;
			shortPath.append(":\\");
			if ((logicDiskHandle = CreateFileA(bigPathForHandle.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE){
				cout << "Error: " << GetLastError() << endl;
			}

			if (!DeviceIoControl(logicDiskHandle, IOCTL_STORAGE_GET_DEVICE_NUMBER, 0, 0, outBuffer, BUFSIZ, NULL, NULL)){
				cout << "Error" << GetLastError() << endl;
				CloseHandle(logicDiskHandle);
			}

			deviceNumber = (STORAGE_DEVICE_NUMBER*)outBuffer;
			
			if (counter == deviceNumber->DeviceNumber) {
				all.QuadPart = 0;

				if (GetDiskFreeSpaceEx(shortPath.c_str(), &free, &all, NULL)) {
					freeHDD.QuadPart += free.QuadPart;
					allHDD.QuadPart += all.QuadPart;
				}
				else {
					CloseHandle(logicDiskHandle);
				}
				cout << symbolOfDisk << ": " << endl << "Free space - " << free.QuadPart / 1024 / 1024 / 1024 << "Gb" << endl <<
					"All space - " << all.QuadPart / 1024 / 1024 / 1024 << "Gb" << endl <<
					"Occupied - " << (all.QuadPart - free.QuadPart) / 1024 / 1024 / 1024 << "Gb" << endl;
			}
			CloseHandle(logicDiskHandle);
		}
	}
	cout.setf(ios::left);
	cout
		<< setw(16) << "All space[Gb]"
		<< setw(16) << "Free space[Gb]"
		<< setw(16) << "Occupied space[Gb]" << endl;

	cout
		<< setw(16) << (driveSize / 1024 / 1024 / 1024)
		<< setw(16) << (freeHDD.QuadPart / 1024 / 1024 / 1024)
		<< setw(16) << ((driveSize - freeHDD.QuadPart) / 1024 / 1024 / 1024)
		<< endl;
}
