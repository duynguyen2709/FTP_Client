#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#include <thread>

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

int createSocket(char *ip);

int main(int argc, char **argv)
{
	//----------------------
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error: %ld\n", iResult);
		return 1;
	}
	/*printf("Enter IP :");
	char ip[255];
	gets_s(ip, 255);*/

	//createSocket(argv[1]);
	char *ip1 = "0.0.0.0";

	std::thread t1(createSocket, ip1);
	if (t1.joinable())
	{
		t1.join();
	}

	return 0;
}
int createSocket(char *ip) {

	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests.
	SOCKET ListenSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
		wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr_in service;
	service.sin_family = AF_INET;

	service.sin_addr.s_addr = INADDR_ANY;

	service.sin_port = htons(52700);

	if (bind(ListenSocket,
		(SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR) {
		wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	//----------------------
	// Listen for incoming connection requests.
	// on the created socket
	if (listen(ListenSocket, 1) == SOCKET_ERROR) {
		wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	//----------------------
	// Create a SOCKET for accepting incoming requests.
	SOCKET AcceptSocket;
	wprintf(L"Waiting for client to connect...\n");

	//----------------------
	// Accept the connection.
	AcceptSocket = accept(ListenSocket, NULL, NULL);

	if (AcceptSocket == INVALID_SOCKET) {
		wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		int iResult;
		char buf[512];
		while ((iResult = recv(AcceptSocket, buf, 512, 0)) > 0) {
			printf("%s", buf);
			memset(buf, 0, iResult);
		}
	}

	system("pause");

	// No longer need server socket
	closesocket(ListenSocket);

	WSACleanup();
	return 0;
}