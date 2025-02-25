//Weel-known ports
			//WEB:TCP-80;
			//DHCP:UDP-67, UDP-68;
			//C:\Windows\System32\drivers\etc\services
			//Socket
			//Winsock2.h
			//#pragma comment(lib, "Ws2_32.lib")
			///////////////////////////////////
			/*
				Server:
				1. Initialize Winsock;
				2. Create a Socket;
				3. Bind Socket;
				4. Listen Socked;
				5. Accept connection;
				6. Receive & Send data;
				7. Disonnect;
			*/

#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include <windows.h>
#include <ctime>

using std::cin;
using std::cout;
using std::endl;

#define DEFAULT_PORT	"27015"
#define BUFFER_SIZE		   1500
#define MAX_CONNECTIONS       3

#pragma comment(lib, "Ws2_32.lib")

void printCurrentDateTime() {
	// �������� ������� �����
	std::time_t now = std::time(0);

	// ����������� � ������ ������� "YYYY-MM-DD HH:MM:SS"
	char* dt = std::ctime(&now);

	// ������� ������� ����� � ����
	std::cout << "������� ���� � �����: " << dt;
}

union ClientSocketData
{
	SOCKADDR client_socket;
	unsigned long long data;
	ClientSocketData(SOCKADDR client_socket)
	{
		this->client_socket = client_socket;

	}
	unsigned long long get_data() const
	{
		return data;
	}

	unsigned short get_port() const
	{
		//int i_port = (data >>16) & 0xFFFF;
		//return i_port;

		return (unsigned char)client_socket.sa_data[0] * 256 +
			(unsigned char)client_socket.sa_data[1];


	}
	char* get_socket(char* sz_client_name) const
	{
		sprintf(
			sz_client_name,
			"%i.%i.%i.%i:%i",
			(unsigned char)client_socket.sa_data[2],
			(unsigned char)client_socket.sa_data[3],
			(unsigned char)client_socket.sa_data[4],
			(unsigned char)client_socket.sa_data[5],
			get_port()
			//(unsigned char)client_socket.sa_data[0] * 256 +
			//(unsigned char)client_socket.sa_data[1]
		);
		return sz_client_name;
	}
};

void HandleClient(LPVOID LParam);
SOCKET ClientSocket;
SOCKET client_sockets[MAX_CONNECTIONS]{};
HANDLE client_handles[MAX_CONNECTIONS]{};
DWORD dw_thread_id[MAX_CONNECTIONS]{};
int* client_number[MAX_CONNECTIONS]{};
int number_of_clients = 0;

void PrintNumberOfClients();
int FindFreeSlot();

void main()
{
	setlocale(LC_ALL, "");

	//1. Initialize WinSock:
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);	//(2,2) - Version
	if (iResult != 0)
	{
		cout << "WinSock initialization failed with error #" << iResult << endl;
		return;
	}

	//2. Create Socket:
	addrinfo* result = NULL;
	addrinfo* ptr = NULL;
	addrinfo hInst;

	//sockaddr* addr = NULL;
	struct sockaddr saServer;
	struct in_addr saServer1;

	//int* addrlen =0;

	//2.1. �������� ����� �������� ����:
	ZeroMemory(&hInst, sizeof(hInst));
	hInst.ai_family = AF_INET;
	hInst.ai_socktype = SOCK_STREAM;
	hInst.ai_protocol = IPPROTO_TCP;
	hInst.ai_flags = AI_PASSIVE;

	iResult = GetAddrInfo(NULL, DEFAULT_PORT, &hInst, &result);
	if (iResult != 0)
	{
		cout << "GetAddrInfo failed with error #" << iResult << endl;
		WSACleanup();
		return;
	}

	//2.2. ������� �����:
	SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		cout << "Socket error: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//3. Binding - ����������� ����� � �����:
	iResult = bind(ListenSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Bind failed with error #" << WSAGetLastError() << endl;
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	freeaddrinfo(result);	//����� ������ ������� bind() ���������� �� ������ ���� ������ �� �����.

	//4. Listen port:
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Listen failed with error #" << WSAGetLastError() << endl;
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}
	cout << "Server started on TCP port " << DEFAULT_PORT << endl;


	//5. Accept connection:
	do
	{
		PrintNumberOfClients();
		CHAR sz_client_name[32];
		int namelen = 32;
		SOCKADDR client_socket;
		ZeroMemory(&client_socket, sizeof(client_socket));

		if (number_of_clients < MAX_CONNECTIONS)
		{
			int i = FindFreeSlot();
			if (client_number[i] == NULL) {
				client_number[i] = (int*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(int));
				*client_number[i] = number_of_clients;
				//client_number[number_of_clients] = (int*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(int));
				//*client_number[number_of_clients] = number_of_clients;
			}
			client_sockets[i] = accept(ListenSocket, (SOCKADDR*)&client_socket, &namelen);//sizeof(saServer)
			/*char buffer[INET_ADDRSTRLEN];
			std::string cidrlong = inet_ntop(AF_INET, &saServer.sa_data, buffer, sizeof(buffer));//AF_INET6
			std::cout << "**** #" << cidrlong << endl;*/

			//cout << "**** #" << addr->sa_family 
			if (ClientSocket == INVALID_SOCKET)
			{
				cout << "Accept failed with error #" << WSAGetLastError() << endl;
				closesocket(ListenSocket);
				//WSACleanup();
				//return;
			}

			//
			// HandleClient(ClientSocket);

			//HANDLE handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HandleClient, NULL, 0, 0);
			client_handles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HandleClient, client_number[i], 0, 0);
			number_of_clients++;
		}
		else
		{
			SOCKET extra_socket = accept(ListenSocket, &client_socket, &namelen);
			char recvextrabuffer[BUFFER_SIZE]{};
			recv(extra_socket, recvextrabuffer, BUFFER_SIZE, 0);
			char message[] = "No free connections left";
			send(extra_socket, message, sizeof(message), 0);
			shutdown(extra_socket, SD_SEND);
			cout << ClientSocketData(client_socket).get_socket(sz_client_name) << " was disconnected";
			closesocket(extra_socket);
		}
		Sleep(100);
	} while (true);
	system("PAUSE");
	WSACleanup();
	system("PAUSE");
}

void HandleClient(LPVOID LParam)
{

	CHAR sz_client_name[32];
	int namelen = 32;
	SOCKADDR client_socket;
	int i = *((int*)LParam);
	//getsockname(ClientSocket, &client_socket, &namelen);
	getpeername(client_sockets[i], &client_socket, &namelen);
	//cout << "getsockname error # " << WSAGetLastError() << endl;
	//cout << client_socket.sa_data << endl;
	//cout << sz_client_name << endl;
	/*sprintf(
		sz_client_name,
		"%i.%i.%i.%i:%i",
		(unsigned char)client_socket.sa_data[2],
		(unsigned char)client_socket.sa_data[3],
		(unsigned char)client_socket.sa_data[4],
		(unsigned char)client_socket.sa_data[5],
		(unsigned char)client_socket.sa_data[0]*256 +
		(unsigned char)client_socket.sa_data[1]
	);*/
	//cout << sz_client_name << endl;

	//ClientSocketData client_data(client_socket);
	//cout << client_data.get_data() << endl;
	//cout << "Client: " << client_data.get_socket(sz_client_name) << endl;
	cout << "Client: " << ClientSocketData(client_socket).get_socket(sz_client_name) << endl;
	//closesocket(ClientSocket);
	//closesocket(ListenSocket);

	//6. Receive & Send data:
	//char recvbuffer[BUFFER_SIZE]{};
	int received = 0;
	do
	{

		char recvbuffer[BUFFER_SIZE]{};
		ZeroMemory(recvbuffer, BUFFER_SIZE);
		received = recv(client_sockets[i], recvbuffer, BUFFER_SIZE, 0);
		if (received > 0)
		{
			cout << "Bytes received:  \t" << received << endl;
			cout << "Message from :\t" << sz_client_name << "\t" << recvbuffer << endl;
			printCurrentDateTime();
			//cout << "Received message:\t" << recvbuffer << endl;
			//int iSendResult = send(ClientSocket, "������ Client", received, 0);
			int iSendResult = send(client_sockets[i], recvbuffer, received, 0);

			if (iSendResult == SOCKET_ERROR)
			{
				cout << "Send failed with error #" << WSAGetLastError() << endl;
				closesocket(client_sockets[i]);
				WSACleanup();
				return;
			}
			cout << "Bytes sent: " << iSendResult << endl;
		}
		else if (received == 0)cout << "Connection closing..." << endl;
		else
		{
			cout << "Receive failed with error #" << WSAGetLastError() << endl;
			closesocket(client_sockets[i]);
			//WSACleanup();
			//return;
		}
	} while (received > 0);

	//7. Disconnection:
	int iResult = shutdown(client_sockets[i], SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		cout << "shutdown failed with error #" << WSAGetLastError() << endl;
	}
	closesocket(client_sockets[i]);
	HANDLE threadHandle = client_handles[i];
	client_sockets[i] = NULL;
	client_handles[i] = NULL;
	number_of_clients--;
	PrintNumberOfClients();
	CloseHandle(threadHandle);

}

void PrintNumberOfClients()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	ZeroMemory(&consoleInfo, sizeof(consoleInfo));
	GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
	SetConsoleCursorPosition(hConsole, COORD{ 85,0 });
	cout << "���������� �������:  " << number_of_clients << endl;

	SetConsoleCursorPosition(hConsole, consoleInfo.dwCursorPosition);
}

int FindFreeSlot()
{
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (client_handles[i] == NULL && client_sockets[i] == NULL) return i;

		return -1;
	}
}