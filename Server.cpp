#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <conio.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

//Static:
static int counterConn = 0;						//������� �����������
static vector<pair<HANDLE,SOCKET>> connections;	//������ �������� �����������
static vector<int> index;						//������ ��������������� �������� �����������
static bool escHandle;							//���� ������� ������� Esc
//--------------------------

// Functions:
void getNum(int* num);
int getPosNumFromWord(char* word);
int getNumFromWord(char* word);
int getNumFromChar(char a);
//--------------------------

// Threads:
//����� ��������� �����������
void ThreadConnection(LPVOID ipParam) {
	SOCKET socket = (SOCKET)ipParam;
	
	//��������� ��������� ������ ��������
	char msg[10] = "A";//Accept
	send(socket, msg, sizeof(msg), NULL);

	//������� ������������� �������
	recv(socket, msg, sizeof(msg), NULL);
	int indexT = getPosNumFromWord(msg);
	index.push_back(indexT);
	cout << "Client " << indexT << " connected. Index " << (index.size() - 1) << "." << endl;
	//����� ����� �� �������
	DWORD timeout = 5000;
	while (true) {
		if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != 0) {
			cout << "MessageTimeoutError" << endl;
		}
		else {
			strcpy_s(msg, sizeof(msg), "");
			int r = recv(socket, msg, sizeof(msg), NULL);
			if (r >= 0) {
				if (msg != "") {
					int num = getNumFromWord(msg);
					cout << "<--" << indexT << " \"" << num << "\"" << endl;
					sprintf_s(msg, "%d", ++num);
					send(socket, msg, sizeof(msg), NULL);
					cout << "-->" << indexT << " \"" << num << "\"" << endl;
				}
			}
			else
				if (r != 0) {
					cout << "Connection " << indexT << " lost." << endl;
					break;
				}
		}
	}

	for (int i = 0; i < connections.size(); i++)
		if (connections[i].second == socket) {
			connections.erase(connections.begin() + i);
			cout << "Client " << indexT << " disconnected" << endl;
			index.erase(index.begin() + i);
			break;
		}
	counterConn--;
	return;
}

//����� ��������� ������� �����������
void ThreadBadConnection(LPVOID ipParam) {
	SOCKET socket = (SOCKET)ipParam;
	//��������� ��������� ������ �����
	char msg = 'D';//Denied
	send(socket, &msg, sizeof(char), NULL);
	return;
}

//���� ������ ������� Esc, ��������� ���������
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
	//����� ������� ���� �������
	HWND hwnd = GetConsoleWindow();
	MoveWindow(hwnd, 0, 0, 450, 500, true);
	//����������� �� ���������� �����������
	int numConn;
	if (argc > 1) {
		numConn = getPosNumFromWord(argv[1]);
		if (numConn < 1)
			getNum(&numConn);
	}
	else
		getNum(&numConn);

	//������ �������
	cout << "Server start!" << endl;
	WSAData wsaData;
	WORD dllVersion = MAKEWORD(2, 1);
	//�������� �� ������ WinSock
	if (WSAStartup(dllVersion, &wsaData) != 0) {
		cout << "Error: WSA won't start" << endl;
		exit(1);
	}

	SOCKADDR_IN addrIn;
	InetPton(AF_INET, L"127.0.0.1", &addrIn.sin_addr.s_addr);
	addrIn.sin_port = htons(7777);
	addrIn.sin_family = AF_INET;
	int addrInSize = sizeof(addrIn);

	SOCKET sockListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sockListen, (SOCKADDR*)&addrIn, addrInSize);
	listen(sockListen, SOMAXCONN);

	//���������� ��������� ��� ������� Esc
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadStop, NULL, NULL, NULL);

	//���������� ��������� ����� �����������, ���� �� ������ ������� Esc
	SOCKET newConn;
	escHandle = false;
	while (!escHandle) {
		newConn = accept(sockListen, (SOCKADDR*)&addrIn, &addrInSize);
		if (newConn == 0)
			cout << "Error connect to " << addrIn.sin_addr.s_addr << endl;
		
		if (newConn > 0) {
			HANDLE hThread;
			if (counterConn < numConn) {
				hThread = (CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadConnection, (LPVOID)newConn, NULL, NULL));
				connections.push_back(make_pair(hThread, newConn));
				counterConn++;
			}
			else {
				hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadBadConnection, (LPVOID)newConn, NULL, NULL);
			}
		}
	}
	if (newConn == 0)
		cout << "Error connect to " << addrIn.sin_addr.s_addr << endl;

	system("pause");
}


//���� ����� ��������� �����������
void getNum(int* num) {
	while (true) {
		cout << "Please enter count clients:";
		cin >> *num;
		if (*num < 1)
			cout << "Incorrect number!" << endl;
		else
			break;
	}
}

//��������� ����� ����������� �� ��������� ���������� ������
int getPosNumFromWord(char* word) {
	int num = 0;
	if ((getNumFromChar(word[0])) != -1)
		for (int i = 0; word[i] != '\0' && word[i] != ' '; i++)
			num = num * 10 + getNumFromChar(word[i]);
	else
		num = -1;
	return num;
}

//��������� ����� �� ������ (�� ������� �������), � �.�. �������������
int getNumFromWord(char* word) {
	int num = 0;
	int sign = 1;
	if (word[0] == '-')
		sign = -1;
	for (int i = 0; word[i] != '\0' && word[i] != ' '; i++)
		num = num * 10 + getNumFromChar(word[i]);
	return num*sign;
}

//��������� ����� �� �������
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
