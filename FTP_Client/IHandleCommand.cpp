#include "stdafx.h"
#include "FTP_Client.h"

vector<int> IHandleCommand::PortUsed = {};

SOCKET IHandleCommand::createListeningSocket(int port)
{
	int iResult;
	SOCKET ListenSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
		wprintf(L"Created Listening Socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return NULL;
	}

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons((u_short)port);

	if (::bind(ListenSocket,
		(SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR) {
		wprintf(L"Bind failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return NULL;
	}

	if (listen(ListenSocket, 1) == SOCKET_ERROR) {
		wprintf(L"Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return NULL;
	}

	return ListenSocket;
}

int IHandleCommand::portCommand()
{
	My_IP_Address ipAddress;
	char buf[BUFSIZ + 1];

	int resCode;

	int port = getNextFreePort();
	int temp = port / 256;

	sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", ipAddress.x1, ipAddress.x2, ipAddress.x3, ipAddress.x4, temp, port - temp * 256);

	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;

	return port;
}

const char * IHandleCommand::formatBuffer(string command, string & srcFileName, string & dstFileName)
{
	if (command == "dir")
		return "LIST\r\n";

	if (command == "ls")
		return "NLST\r\n";

	//check if parameters' existence
	int pos = command.find_first_of(' ');
	int pos2 = command.find(' ', pos + 1);

	if (command.find("get") != string::npos) {

		//no parameter
		if (pos == string::npos) {
			cout << "Remote file ";
			getline(cin, srcFileName);

			cout << "Local file ";
			getline(cin, dstFileName);
		}
		else {

			//1 parameter
			if (pos2 == string::npos) {
				srcFileName = dstFileName = command.substr(pos + 1);
			}

			//2 parameter
			else {
				srcFileName = command.substr(pos + 1, pos2 - pos - 1);
				dstFileName = command.substr(pos2 + 1);
			}
		}

		char temp[BUFSIZ];
		sprintf(temp, "RETR %s\r\n", srcFileName.c_str());
		return temp;
	}

	if (command.find("put") != string::npos) {

		//no parameter
		if (pos == string::npos) {
			cout << "Local file ";
			getline(cin, srcFileName);

			cout << "Remote file ";
			getline(cin, dstFileName);
		}
		else
		{
			//1 parameter
			if (pos2 == string::npos) {
				srcFileName = dstFileName = command.substr(pos + 1);
			}

			//2 parameter
			else {
				srcFileName = command.substr(pos + 1, pos2 - pos - 1);
				dstFileName = command.substr(pos2 + 1);
			}
		}

		if (ifstream(srcFileName).good())
		{
			char temp[BUFSIZ];
			sprintf(temp, "STOR %s\r\n", dstFileName.c_str());
			return temp;
		}
		else {
			cout << srcFileName << ": File not found" << endl;
			return "";
		}
	}
}

void IHandleCommand::portRelatedCommands(string command)
{
	char buf[BUFSIZ + 1];

	int resCode;

	int port = portCommand();

	SOCKET ListenSocket = createListeningSocket(port);

	string srcFileName, dstFileName;

	strcpy_s(buf, formatBuffer(command, srcFileName, dstFileName));
	if (strcmp(buf, "") == 0)
		return;

	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;

	SOCKET AcceptSocket;
	AcceptSocket = accept(ListenSocket, NULL, NULL);

	if (AcceptSocket == INVALID_SOCKET) {
		wprintf(L"Accept failed with error: %ld\n", WSAGetLastError());
	}
	else
	{
		int iResult;

		//DIR/LS Command
		if (command == "dir" || command == "ls") {
			while ((iResult = recv(AcceptSocket, buf, BUFSIZ, 0)) > 0) {
				cout << buf;
				memset(buf, 0, iResult);
			}
		}

		//GET Command
		else if (command.find("get") != string::npos)
			get(AcceptSocket, dstFileName);

		//PUT Command
		else if (command.find("put") != string::npos)
			put(AcceptSocket, srcFileName);
	}

	closesocket(ListenSocket);
	closesocket(AcceptSocket);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;
}

void IHandleCommand::get(SOCKET AcceptSocket, string dstFileName)
{
	int iResult;
	char buf[BUFSIZ + 1];

	ofstream outputFile;
	outputFile.open(dstFileName.c_str(), ios::out | ios::binary | ios::trunc);

	while ((iResult = recv(AcceptSocket, buf, BUFSIZ, 0)) > 0) {
		outputFile.write(buf, iResult);
		memset(buf, 0, iResult);
	}

	outputFile.close();
}

void IHandleCommand::put(SOCKET AcceptSocket, string srcFileName)
{
	char buf[BUFSIZ + 1];

	ifstream inputFile;
	inputFile.open(srcFileName.c_str(), ios::binary | ios::in);

	//get file's size
	inputFile.seekg(0, ios_base::end);
	int length = inputFile.tellg();
	inputFile.seekg(0, ios_base::beg);

	while (length > BUFSIZ)
	{
		inputFile.read(buf, BUFSIZ);
		send(AcceptSocket, buf, BUFSIZ, 0);
		length -= BUFSIZ;
	}

	inputFile.read(buf, length);
	send(AcceptSocket, buf, length, 0);

	inputFile.close();
}

void IHandleCommand::lcd(string command)
{
	char buf[BUFSIZ];

	//If there's no argument,
	//Then set local directory to user default home directory
	if (command.find_first_of(' ') == string::npos) {
		_chdir(getenv("USERPROFILE"));
		cout << "Local directory now " << getcwd(buf, BUFSIZ) << endl;
		return;
	}

	//
	//else
	//check if directory exists
	//then change to new directory
	//else print not found directory
	//
	string newDir = command.substr(4);

	wstring stemp = wstring(newDir.begin(), newDir.end());
	LPCWSTR dir = stemp.c_str();

	DWORD dwAttrib = GetFileAttributes(dir);

	if (!(dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))) {
		cout << newDir << ": File not found" << endl;
		return;
	}

	if (_chdir(newDir.c_str())) {

		//handling error
		switch (errno)
		{
		case ENOENT:
			cout << "Unable to locate the directory: " << newDir << endl;
			break;
		case EINVAL:
			cout << "Invalid buffer." << endl;
			break;
		default:
			cout << "Unknown error." << endl;
		}
		return;
	}

	cout << "Local directory now " << getcwd(buf, BUFSIZ) << endl;
}

void IHandleCommand::pwd()
{
	char buf[255];

	int resCode;

	sprintf(buf, "XPWD\r\n");

	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);

	cout << buf;
}

void IHandleCommand::directoryCommands(string command, string noti, const int commandLength, const char * format)
{
	char buf[BUFSIZ + 1];

	string dir;

	int resCode = 0;

	int space = command.find_first_of(' ');

	//if no argument then ask for directory name
	if (space == string::npos) {
		cout << noti;
		getline(cin, dir);
	}
	else {
		dir = command.substr(commandLength + 1);
	}

	//reformat directory
	int spaceCount = ::count(dir.begin(), dir.end(), ' ');
	if (spaceCount >= 1)
	{
		int pos = dir.find_first_of(' ');
		dir = dir.substr(0, pos);
	}

	//send command & receive response
	sprintf(buf, format, dir.c_str());
	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);

	cout << buf;
}