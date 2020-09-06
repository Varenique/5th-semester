#include "Com.h"

void Server(char* path);
void Client();

int main(int argc, char* argv[])
{
	switch (argc)
	{
	case 1:
		Server(argv[0]);
		break;
	default:
		Client();
		break;
	}
}

int enterBaud() {

	int baud;
	cout << "Enter baud: " << endl;
	cin >> baud;
	if (cin.peek() == '\n') {
		cin.ignore();
	}
	return baud;
}

void Server(char* path) {

	HANDLE semaphoreBaud = CreateSemaphore(NULL, 0, 1, "BAUD");
	string portName = "COM3";
	string writeString;
	char buffer[BUFFER_SIZE];

	cout << "Server start working (COM1)\n";
	
	Com com1 = Com();
	com1.init_port(portName, enterBaud());
	
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(								// создаем новый процесс для клиента
		(LPSTR)path,//путь для создания нового процесса
		(LPSTR)" Client",//имя процесса
		NULL,// атрибуты защиты для нового процесса
		NULL,//атрибуты защиты для первичного потока
		FALSE,//флаг наследования
		CREATE_NEW_CONSOLE,//процесс создаётся в новой консоли 
		NULL,//переменные окружения
		NULL,//текущий диск и каталог.
		&si,
		&pi
	)) {
		CloseHandle(com1.COMport);
		cout << "Error during creating client process" << endl;
		return;
	}

	COMMTIMEOUTS timeouts;//структура для установки интервалов простоя
	timeouts.ReadIntervalTimeout = 0xFFFFFFFF;// Mаксимальный интервал простоя между поступлением двух символов по линии связи
	timeouts.ReadTotalTimeoutMultiplier = 0;//Множитель, используемый, чтобы вычислить полный период времени простоя для операций чтения
	timeouts.ReadTotalTimeoutConstant = CONSTANT_TIMEOUT;//Константа, используемая, чтобы вычислить полный период времени простоя для операций чтения
	timeouts.WriteTotalTimeoutMultiplier = 0;// Множитель, используемый, чтобы вычислить полный период времени простоя для операций записи
	timeouts.WriteTotalTimeoutConstant = CONSTANT_TIMEOUT;// Константа, используемая, чтобы вычислить полный период времени простоя для операций записи

	if (!SetCommTimeouts(com1.COMport, &timeouts)) {//установка интервалов простоя
		CloseHandle(com1.COMport);
		cout << "Error during setting timeouts" << endl;
		return;
	}
	
	WaitForSingleObject(semaphoreBaud, INFINITE);//ждём сигнала от ридера
	

	while (true)
	{
		DWORD writtenBytes = 0;

		cout << "Enter string to pass: ";
		cin.clear();
		getline(cin, writeString);
		
		int portionNumber = writeString.size() / BUFFER_SIZE + 1;//столько раз будут передаваться данные из буфера
																   //отправляем 2 порту

		com1.write(LPCVOID(&portionNumber), sizeof(portionNumber));

		int size = writeString.size();
		com1.write(LPCVOID(&size), sizeof(size));
		//отправляем 2 порту размер введённой строки

		for (int i = 0; i < portionNumber; i++) {
			writeString.copy(buffer, BUFFER_SIZE, i * BUFFER_SIZE);//копируем в буфер части введённой строки
			com1.write(LPCVOID(buffer), BUFFER_SIZE);
		}	
	}

	com1.disconnect();
	CloseHandle(semaphoreBaud);

	return;
}



void Client()
{
	string portName = "COM4";
	string readString;
	char buffer[BUFFER_SIZE];
	HANDLE semaphoreBaud = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "BAUD");

	cout << "Client start working(COM2)\n";
	
	Com com2 = Com();
	com2.init_port(portName, enterBaud());

	ReleaseSemaphore(semaphoreBaud, 1, NULL);//освобождение счётчика
	
	while (true)
	{
		DWORD readBytes;
		readString.clear();


		int portionNumber;
		com2.read(&portionNumber, sizeof(portionNumber));

		int size;
		com2.read(&size, sizeof(size));


		for (int i = 0; i < portionNumber; i++)
		{
			com2.read(buffer, BUFFER_SIZE);
			readString.append(buffer, BUFFER_SIZE);//расширяет строку
		}

		readString.resize(size);//действительный размер стзоки

		cout << "COM2 get: " << endl;
		for (int i = 0; i < size; i++)
		{
			cout << readString[i];
		}
		cout << endl;

	}

	com2.disconnect();
	return;
}
