#include "Com.h"


Com::Com() {
	COMport = 0;
}

void Com::init_port(const std::string& port, int baud)
{

	COMport = CreateFile(
		(LPSTR)port.c_str(), //наименование порта, например COM1
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
	//устанавливаем размеры входного и выходного буферов
	SetupComm(COMport, 1500, 1500);
	SetCommMask(COMport, EV_RXCHAR);//определение отслеживаемых событий(символ был принят и помещён в буфер ввода данных)



	
	memset(&dcb, 0, sizeof(dcb));//выделение памяти под dcb
	dcb.DCBlength = sizeof(DCB);//длина структуры в байтах
	if (!GetCommState(COMport, &dcb)) {
		CloseHandle(COMport);
		std::cout << "Error during DCB initialize" << std::endl;
		return;
	}

	dcb.BaudRate = DWORD(baud); //скорость передачи данных в бодах
	dcb.fBinary = TRUE;//двоичный режим
	dcb.fParity = FALSE;//режим чётности
	dcb.fOutxCtsFlow = FALSE;// проверяется сигнал готовности к приему (CTS)
	dcb.fOutxDsrFlow = FALSE;// проверяется сигнал готовности модема (DSR)
	dcb.fDtrControl = DTR_CONTROL_DISABLE;//Сигнал  DTR (готовности терминала к передаче данных)
	dcb.fInX = FALSE;//используется ли XON/XOFF управление потоком данных в ходе приема
	dcb.fOutX = FALSE;//используется ли XON/XOFF управление потоком данных в ходе передачи	
	dcb.fNull = FALSE;// при приеме пустые байты не сбрасываются
	dcb.fRtsControl = RTS_CONTROL_TOGGLE;//Сигнал RTS ( готовности к передаче) 
	dcb.fAbortOnError = TRUE;//драйвер завершает все операции чтения и записи с состоянием ошибки, если происходит ошибка
	dcb.XonLim = 255;//Минимальное число байтов, которое допустимо в буфере ввода данных
	dcb.XoffLim = 255;//Максимальное число байтов, допустимое в буфере ввода данных
	dcb.ByteSize = 8; //число информационных бит в передаваемых и принимаемых байтах.
	dcb.Parity = NOPARITY;//нет проверки чётности, паритет
	dcb.StopBits = ONESTOPBIT;//1 стоповый бит	
	dcb.XonChar = 0; // значение XON 
	dcb.XoffChar = 255;// величина XOFF

	if (!SetCommState(COMport, &dcb)) {//установка конфигурации
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