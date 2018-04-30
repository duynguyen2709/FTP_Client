// ftp_clnt_csocket.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "FTP_Client.h"
#include <direct.h>

vector<string> FTP_Client::CommandList = {};
vector<int> IHandleCommand::PortUsed = {};
vector<pair<int, string>> ResponseErrorException::ErrorCodeList = {};

bool FTP_Client::isLegitIPAddress(string command)
{
	string IP_Server;

	if (command == "open" || command == "ftp")
	{
		cout << "To :";
		getline(cin, IP_Server);
	}
	else
	{
		int pos = command.find_first_of(' ');
		IP_Server = command.substr(pos + 1);
	}

	regex ipAddressFormat("(^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])$)");

	if (!regex_match(IP_Server, ipAddressFormat))
		return false;

	return true;
}

Command FTP_Client::commandValue(string command)
{
	Command cmd;

	if (command.find("ls") != string::npos)
	{
		cmd = LS;
	}
	else if (command.find("mkdir") != string::npos)
	{
		cmd = MKDIR;
	}
	else if (command.find("rmdir") != string::npos)
	{
		cmd = RMDIR;
	}
	else if (command.find("dir") != string::npos)
	{
		cmd = DIR;
	}
	else if (command.find("mput") != string::npos)
	{
		cmd = MPUT;
	}
	else if (command.find("put") != string::npos)
	{
		cmd = PUT;
	}
	else if (command.find("mget") != string::npos)
	{
		cmd = MGET;
	}
	else if (command.find("get") != string::npos)
	{
		cmd = GET;
	}
	else if (command.find("lcd") != string::npos)
	{
		cmd = LCD;
	}
	else if (command.find("cd") != string::npos)
	{
		cmd = CD;
	}
	else if (command.find("mdelete") != string::npos)
	{
		cmd = MDELETE;
	}
	else if (command.find("delete") != string::npos)
	{
		cmd = _DELETE;
	}

	else if (command.find("pwd") != string::npos)
	{
		cmd = PWD;
	}
	else if (command.find("passive") != string::npos)
	{
		cmd = PASSIVE;
	}

	return cmd;
}

FTP_Client::FTP_Client()
{
	ConnectionStatus = false;
	CommandHandler = static_cast<IHandleCommand *>(&(*this));
}

FTP_Client::~FTP_Client()
{
	ClientSocket.Close();
}

bool FTP_Client::Login(string command)
{
	if (!isLegitIPAddress(command))
	{
		cout << "Invalid IP Address." << endl;
		return false;
	}

	ResponseErrorException ex;
	try
	{
		// Tao socket dau tien
		ClientSocket.Create();

		// Ket noi den Server
		if (ClientSocket.Connect(_T(server), 21) == 0)
			return false;

		char buf[BUFSIZ + 1];
		int tmpres, size;
		/*
		Connection Establishment
		120
		220
		220
		421
		Login
		USER
		230
		530
		500, 501, 421
		331, 332
		PASS
		230
		202
		530
		500, 501, 503, 421
		332
		*/
		char * str;
		int codeftp;

		//cout << "Connection established, waiting for welcome message...\n";

		memset(buf, 0, sizeof buf);
		while ((tmpres = ClientSocket.Receive(buf, BUFSIZ, 0)) > 0) {
			sscanf(buf, "%d", &codeftp);
			cout << buf;
			if (codeftp != 220) //120, 240, 421: something wrong
			{
				ex.setErrorCode(codeftp);
				throw ex;
			}

			str = strstr(buf, "220");
			if (str != NULL) {
				break;
			}
			memset(buf, 0, tmpres);
		}

		//Send Username
		char info[50];
		cout << "Name (" << server << ") :";
		memset(buf, 0, sizeof buf);
		scanf("%s", info);

		sprintf(buf, "USER %s\r\n", info);
		tmpres = ClientSocket.Send(buf, strlen(buf), 0);

		memset(buf, 0, sizeof buf);
		tmpres = ClientSocket.Receive(buf, BUFSIZ, 0);

		sscanf(buf, "%d", &codeftp);
		if (codeftp != 331)
		{
			ex.setErrorCode(codeftp);
			throw ex;
		}
		cout << buf;

		//Send Password
		//Hide current text typing
		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
		DWORD mode = 0;
		GetConsoleMode(hStdin, &mode);
		SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));

		memset(info, 0, sizeof info);
		cout << "Password: ";
		memset(buf, 0, sizeof buf);
		scanf("%s", info);

		//End hiding text typing
		SetConsoleMode(hStdin, mode);

		sprintf(buf, "PASS %s\r\n", info);
		tmpres = ClientSocket.Send(buf, strlen(buf), 0);

		memset(buf, 0, sizeof buf);
		tmpres = ClientSocket.Receive(buf, BUFSIZ, 0);

		sscanf(buf, "%d", &codeftp);
		if (codeftp != 230)
		{
			ex.setErrorCode(codeftp);
			throw ex;
		}
		cout << endl << buf;
		ConnectionStatus = true;

		cin.ignore();
		cin.clear();

		return true;
	}
	catch (ResponseErrorException &e)
	{
		ConnectionStatus = false;
		cout << e.getErrorStringResponse() << endl;
		return false;
	}

	return false;
}

void FTP_Client::InitCommandList()
{
	CommandList.push_back("ftp");
	CommandList.push_back("open");
	CommandList.push_back("ls");
	CommandList.push_back("dir");
	CommandList.push_back("put");
	CommandList.push_back("get");
	CommandList.push_back("mput");
	CommandList.push_back("mget");
	CommandList.push_back("cd");
	CommandList.push_back("lcd");
	CommandList.push_back("delete");
	CommandList.push_back("mdelete");
	CommandList.push_back("mkdir");
	CommandList.push_back("rmdir");
	CommandList.push_back("pwd");
	CommandList.push_back("passive");
	CommandList.push_back("quit");
	CommandList.push_back("exit");
}

bool FTP_Client::checkCommand(string command)
{
	for (auto cmd : CommandList)
		if (command == cmd)
		{
			return true;
		}
	return false;
}

void FTP_Client::ExecuteCommand(string command)
{
	Command cmd = commandValue(command);

	switch (cmd)
	{
	case LS:
	case DIR:
		CommandHandler->dir(command);
		break;
	case PUT:
		break;
	case GET:
		break;
	case MPUT:
		break;
	case MGET:
		break;
	case CD:
		CommandHandler->serverSideCommands(command, "Remote directory:", 2, "CWD %s\r\n");
		break;
	case LCD:
		CommandHandler->lcd(command);
		break;
	case _DELETE:
		CommandHandler->serverSideCommands(command, "Remote file: ", 4, "DELE %s\r\n");
		break;
	case MDELETE:
		break;
	case MKDIR:
		CommandHandler->serverSideCommands(command, "Directory name: ", 5, "XMKD %s\r\n");
		break;
	case RMDIR:
		CommandHandler->serverSideCommands(command, "Directory name: ", 5, "XRMD %s\r\n");
		break;
	case PWD:
		CommandHandler->pwd();
		break;
	case PASSIVE:
		break;

	default:
		break;
	}
	return;
}

void ResponseErrorException::InitErrorCodeList()
{
	ErrorCodeList.push_back(make_pair(0, "Not connected"));
	ErrorCodeList.push_back(make_pair(1, "Invalid command"));
	ErrorCodeList.push_back(make_pair(200, "Command okay"));
	ErrorCodeList.push_back(make_pair(202, "Command not implemented, superfluous at this site."));
	ErrorCodeList.push_back(make_pair(421, "Service not available, closing control connection."));
	ErrorCodeList.push_back(make_pair(500, "Syntax error, command unrecognized.This may include errors such as command line too long."));
	ErrorCodeList.push_back(make_pair(501, "Syntax error in parameters or arguments."));
	ErrorCodeList.push_back(make_pair(502, "Command not implemented."));
	ErrorCodeList.push_back(make_pair(503, "Bad sequence of commands."));
	ErrorCodeList.push_back(make_pair(530, "Not logged in."));
	ErrorCodeList.push_back(make_pair(550, "Requested action not taken."));
}

SOCKET IHandleCommand::createListeningSocket(int port)
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error: %ld\n", iResult);
		return NULL;
	}

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

	int port = randomPort();
	int temp = port / 256;

	sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", ipAddress.x1, ipAddress.x2, ipAddress.x3, ipAddress.x4, temp, port - temp * 256);

	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;

	return port;
}

void IHandleCommand::dir(string command)
{
	char buf[BUFSIZ + 1];

	int resCode;

	int port = portCommand();

	SOCKET ListenSocket = createListeningSocket(port);

	if (command == "dir")
		sprintf(buf, "LIST\r\n");
	else if (command == "ls")
		sprintf(buf, "NLST\r\n");

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
		while ((iResult = recv(AcceptSocket, buf, BUFSIZ, 0)) > 0) {
			cout << buf;
			memset(buf, 0, iResult);
		}
	}

	closesocket(ListenSocket);
	closesocket(AcceptSocket);

	//WSACleanup();

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;
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

void IHandleCommand::serverSideCommands(string command, string noti, const int commandLength, const char * format)
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