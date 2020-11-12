
#include <iostream>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <ctime>
#include <thread>
#include <vector>




#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#define paramThread1 "10100011010011"
#define paramThread2 "11100101101101"
#define MAX_TRIES 5
#define SIZE 14




using namespace std;
using namespace std::chrono;

bool isBusy = false;
bool jam = false;
vector<int> channel;
int counter = 0;

void sendInfo(char* infoToChannel) {
	
	srand(GetCurrentThreadId());
	int delay = (1 + rand() % (100 - 1)) * 5;
	Sleep(delay);

	for (int i = 0; i < SIZE; i++) {
		
		channel.push_back((int)infoToChannel[i] - '0');
		Sleep(100);
	}
}


void Thread(void* pParams) {
	char* info = (char*)pParams;
	int delay = 0;

	while (isBusy);
	if (jam) {
		srand(GetCurrentThreadId());

		delay = (1 + rand() % (3 - 1)) * 50;
		
		Sleep(delay);
		cout << "Delay: " << delay << endl;
		jam = false;
		while (isBusy);

	}
	
	cout << "thread start" << endl;
	isBusy = true;
	sendInfo(info);
	isBusy = false;

}

bool isCollision() {
	bool collision = false;
	int count = 0;
	if (channel.size() == 28) {
		count += (channel[10] + channel[0] + channel[1] + channel[3] + channel[4] + channel[6] + channel[8]) % 2;
		count += (channel[11] + channel[0] + channel[2] + channel[3] + channel[5] + channel[6] + channel[9]) % 2;
		count += (channel[12] + channel[1] + channel[2] + channel[5] + channel[6] + channel[7]) % 2;
		count += (channel[13] + channel[4] + channel[5] + channel[6]) % 2;

		count += (channel[24] + channel[14] + channel[15] + channel[17] + channel[18] + channel[20] + channel[22]) % 2;
		count += (channel[25] + channel[14] + channel[16] + channel[17] + channel[19] + channel[20] + channel[23]) % 2;
		count += (channel[26] + channel[15] + channel[16] + channel[19] + channel[20] + channel[21]) % 2;
		count += (channel[27] + channel[18] + channel[19] + channel[20]) % 2;
	}
	else {
		collision = true;
	}
	if (count != 0) {
		collision = true;
	}

	return collision;
}

void showChannel() {
	cout << "CHANNEL:" << endl;
	for (int i = 0; i < channel.size(); i++) {
		cout << channel[i];
		if (i == SIZE - 1) {
			cout << endl;
		}
	}
	cout << endl;
	cout << "--------------------" << endl;
}



int main() {
	HANDLE hThread1, hThread2, hThread3;
	int answer = 1;
	int collisionWindow = 5000;
	int end = 0;

	
	
	cout << "DATA:\n10100011010011\n11100101101101" << endl;
	cout << "--------------------" << endl;

	while (answer) {
		if (answer != 2) {
			jam = false;
			counter = 0;
		}
		
		channel.clear();
		answer = 1;
		hThread1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread, (LPVOID)"10100011010011", 0, NULL);
		hThread2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread, (LPVOID)"11100101101101", 0, NULL);


		isBusy = false;

		Sleep(collisionWindow);
		showChannel();
		if (isCollision()) {
			jam = true;
			counter++;
			cout << "trial number " << counter << endl << endl;
			
			if (counter < MAX_TRIES) {
				answer = 2;
			}
			else {
				cout << "Too much errors. End (Error)" << endl;
			}
		}
		else {
			cout << "Good send. End." << endl;
			
		}

		CloseHandle(hThread1);
		CloseHandle(hThread2);

		if (answer != 2) {
			cout << "Send again?\n1-yes\n0-no" << endl;
			cin >> answer;
		}
	}
	

	getchar();
	return 0;
}
