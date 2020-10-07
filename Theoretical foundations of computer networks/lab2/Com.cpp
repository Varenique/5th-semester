#include "Com.h"


Com::Com() {
	COMport = 0;
}

void Com::init_port(const std::string& port, int baud)
{

	COMport = CreateFile(
		(LPSTR)port.c_str(), //������������ �����, �������� COM1
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (COMport == INVALID_HANDLE_VALUE) {
		std::cout << "Error during opening COM port" << std::endl;
		return;
	}
	//������������� ������� �������� � ��������� �������
	SetupComm(COMport, 1500, 1500);
	SetCommMask(COMport, EV_RXCHAR);//����������� ������������� �������(������ ��� ������ � ������� � ����� ����� ������)



	
	memset(&dcb, 0, sizeof(dcb));//��������� ������ ��� dcb
	dcb.DCBlength = sizeof(DCB);//����� ��������� � ������
	if (!GetCommState(COMport, &dcb)) {
		CloseHandle(COMport);
		std::cout << "Error during DCB initialize" << std::endl;
		return;
	}

	dcb.BaudRate = DWORD(baud); //�������� �������� ������ � �����
	dcb.fBinary = TRUE;//�������� �����
	dcb.fParity = FALSE;//����� ��������
	dcb.fOutxCtsFlow = FALSE;// ����������� ������ ���������� � ������ (CTS)
	dcb.fOutxDsrFlow = FALSE;// ����������� ������ ���������� ������ (DSR)
	dcb.fDtrControl = DTR_CONTROL_DISABLE;//������  DTR (���������� ��������� � �������� ������)
	dcb.fInX = FALSE;//������������ �� XON/XOFF ���������� ������� ������ � ���� ������
	dcb.fOutX = FALSE;//������������ �� XON/XOFF ���������� ������� ������ � ���� ��������	
	dcb.fNull = FALSE;// ��� ������ ������ ����� �� ������������
	dcb.fRtsControl = RTS_CONTROL_TOGGLE;//������ RTS ( ���������� � ��������) 
	dcb.fAbortOnError = TRUE;//������� ��������� ��� �������� ������ � ������ � ���������� ������, ���� ���������� ������
	dcb.XonLim = 255;//����������� ����� ������, ������� ��������� � ������ ����� ������
	dcb.XoffLim = 255;//������������ ����� ������, ���������� � ������ ����� ������
	dcb.ByteSize = 8; //����� �������������� ��� � ������������ � ����������� ������.
	dcb.Parity = NOPARITY;//��� �������� ��������, �������
	dcb.StopBits = ONESTOPBIT;//1 �������� ���	
	dcb.XonChar = 0; // �������� XON 
	dcb.XoffChar = 255;// �������� XOFF

	if (!SetCommState(COMport, &dcb)) {//��������� ������������
		CloseHandle(COMport);
		std::cout << "Error during setting configuration" << std::endl;
		return;
	}
}


void Com::disconnect()
{
	if (COMport != 0) {
		CloseHandle(COMport);
		COMport = 0;
	}
}

void Com::write(LPCVOID data, int datasize)
{
	if (COMport == 0) {
		return;
	}
	DWORD feedback;
	if (!WriteFile(COMport, data, datasize, &feedback, 0) ||
		feedback != datasize) {
		CloseHandle(COMport);
		COMport = 0;
		return;
	}
}

void Com::read(LPVOID data, int bufsize) {
	DWORD feedback = 0;
	if (!ReadFile(COMport, data, bufsize, &feedback, NULL))
	{
		CloseHandle(COMport);
		COMport = 0;
		return;
	}
}