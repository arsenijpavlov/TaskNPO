#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <conio.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")
#define REPEAT_TIMER 20000
#define LOST_TIMER 10000

using namespace std;

//Static:
static bool escHandle;

//Funstions:
int getNumFromChar(char a);
int getPosNumFromWord(char* word);

//Theards:
//Если нажата клавиша Esc, завершить программу
void ThreadStop() {
	while (true) {
		if (_getch() == 27) {
			escHandle = true;
			break;
		}
	}
	exit(0);
}
//--------------------------

int main(int argc, char* argv[]) {
	//Задаём размеры окна консоли
	HWND hwnd = GetConsoleWindow();
	RECT r;
	GetWindowRect(hwnd, &r);
	MoveWindow(hwnd, r.right - 100, 0, 400, 500, true);
	//Получение параметров из командной строки либо запрос ввода
	int index, sendDelay;
	if (argc > 1) {
		index = stoi(argv[1]);
		if (argc > 2) {
			sendDelay = getPosNumFromWord(argv[2]);
		}
		else {
			cout << "Enter send delay: ";
			cin >> sendDelay;
		}
	}
	else {
		cout << "Enter index: ";
		cin >> index;
		cout << "Enter send delay: ";
		cin >> sendDelay;
	}
	while (index < 9999 || index>99999) {
		cout << "Enter index: ";
		cin >> index;
	}

	cout << "Client start!" << endl;

	WSAData wsaData;
	WORD dllVersion = MAKEWORD(2, 1);
	//проверка на запуск WinSock
	if (WSAStartup(dllVersion, &wsaData) != 0) {
		cout << "Error: WSA won't start" << endl;
		exit(1);
	}

	SOCKADDR_IN addrIn;
	InetPton(AF_INET, L"127.0.0.1", &addrIn.sin_addr.s_addr);
	addrIn.sin_port = htons(7777);
	addrIn.sin_family = AF_INET;
	int addrInSize = sizeof(addrIn);

	//Завершение программы при нажатии Esc
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadStop, NULL, NULL, NULL);

	SOCKET conn;
	escHandle = false;
	int ret;
	while (!escHandle) {
		conn = socket(AF_INET, SOCK_STREAM, NULL);
		ret = connect(conn, (SOCKADDR*)&addrIn, addrInSize);
		if (ret != 0) {
			cout << "Error connection. Repeat after " << REPEAT_TIMER / 1000 << "sec." << endl;
			Sleep(REPEAT_TIMER);
			continue;
		}
		char msg[10];
		//Сервер свободен?
		recv(conn, msg, sizeof(msg), NULL);
		if (msg[0] == 'A') {
			//отправить свой идентификатор
			cout << "Server connected" << endl;
			sprintf_s(msg, "%d", index);
			send(conn, msg, sizeof(msg), NULL);
			//отправить число
			while (true) {
				int num = rand() % 1000;
				sprintf_s(msg, "%d", num);
				int r = send(conn, msg, sizeof(msg), NULL);
				if (r >= 0) {
					cout << "<-- " << num << endl;
					//таймер для проверки соединения
					DWORD timeout = 1500;
					if (setsockopt(conn, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != 0) {
						cout << "MessageTimeoutError" << endl;
					}
					else {
						r = recv(conn, msg, sizeof(msg), NULL);
						if (r >= 0) {
							cout << "--> " << msg << endl;
							Sleep(sendDelay);
						}
					}
				}
				if (r <= 0) {
					cout << "Connection lost." << endl;
					Sleep(LOST_TIMER);
					break;
				}
			}
		}
		else {
			//попробовать снова, если сервер занят
			cout << "Server is full" << endl;
			Sleep(REPEAT_TIMER);
		}
	}

	return 0;
}

//Получение положительного числа из аргумента коммандной строки
int getPosNumFromWord(char* word) {
	int num = 0;
	if ((getNumFromChar(word[0])) != -1)
		for (int i = 0; word[i] != '\0' && word[i] != ' '; i++)
			num = num * 10 + getNumFromChar(word[i]);
	else
		num = -1;
	return num;
}

//Получение цифры из символа
int getNumFromChar(char a) {
	switch (a) {
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	default:
		return -1;
	}
}