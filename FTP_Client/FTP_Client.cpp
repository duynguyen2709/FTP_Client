#include "stdafx.h"
#include "FTP_Client.h"

vector<string> FTP_Client::CommandList = {};

FTP_Client::FTP_Client()
{
	ConnectionStatus = false;
	CommandHandler = static_cast<IHandleCommand *>(&(*this));

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error: %ld\n", iResult);
	}
}

FTP_Client::~FTP_Client()
{
	ClientSocket.Close();
}

bool FTP_Client::checkLegitIPAddress(string command)
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

	if (regex_match(IP_Server, ipAddressFormat))
		return true;

	string ipFromHostName = resolveDomainToIP(IP_Server);

	if (ipFromHostName != "")
	{
		if (regex_match(ipFromHostName, ipAddressFormat))
			return true;
	}
	else cout << "Invalid IP/Host Address" << endl;

	return false;
}

string FTP_Client::resolveDomainToIP(string host)
{
	DWORD dwError;

	struct hostent *remoteHost;
	struct in_addr addr;

	remoteHost = gethostbyname(host.c_str());

	if (remoteHost == NULL)
	{
		dwError = WSAGetLastError();
		if (dwError != 0) {
			return "";
		}
	}
	else
	{
		addr.s_addr = *(u_long *)remoteHost->h_addr_list[0];
		return (string(inet_ntoa(addr)));
	}
	return "";
}

Command FTP_Client::getCommandValue(string command)
{
	Command cmd = _NULL;
	ResponseErrorException ex;

	if (std::count(command.begin(), command.end(), ' ') > 2) {
		ex.setErrorCode(501);
		throw ex;
	}

	int pos = command.find_first_of(' ');
	if (pos != string::npos) {
		command = command.substr(0, pos);
	}

	if (command == "ls")
	{
		if (pos == string::npos)
			cmd = LS;
		else {
			ex.setErrorCode(501);
			throw ex;
		}
	}
	else if (command == "mkdir")
	{
		cmd = MKDIR;
	}
	else if (command == "rmdir")
	{
		cmd = RMDIR;
	}
	else if (command == "dir")
	{
		if (pos == string::npos)
			cmd = DIR;
		else {
			ex.setErrorCode(501);
			throw ex;
		}
	}
	else if (command == "mput")
	{
		cmd = MPUT;
	}
	else if (command == "put")
	{
		cmd = PUT;
	}
	else if (command == "mget")
	{
		cmd = MGET;
	}
	else if (command == "get")
	{
		cmd = GET;
	}
	else if (command == "lcd")
	{
		if (pos == string::npos)
			cmd = LCD;
		else {
			ex.setErrorCode(501);
			throw ex;
		}
	}
	else if (command == "cd")
	{
		cmd = CD;
	}
	else if (command == "mdelete")
	{
		cmd = MDELETE;
	}
	else if (command == "delete")
	{
		cmd = _DELETE;
	}

	else if (command == "pwd")
	{
		if (pos == string::npos)
			cmd = PWD;
		else {
			ex.setErrorCode(501);
			throw ex;
		}
	}
	else if (command == "passive")
	{
		if (pos == string::npos)
			cmd = PASSIVE;
		else {
			ex.setErrorCode(501);
			throw ex;
		}
	}

	return cmd;
}

bool FTP_Client::login(string command)
{
	if (!checkLegitIPAddress(command))
		return false;

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

		char * str;
		int codeftp;

		memset(buf, 0, sizeof buf);
		while ((tmpres = ClientSocket.Receive(buf, BUFSIZ, 0)) > 0) {
			sscanf(buf, "%d", &codeftp);
			cout << buf;
			if (codeftp != 220)
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
		ClientSocket.Close();
		ConnectionStatus = false;

		cin.ignore();
		cin.clear();
		return false;
	}

	return false;
}

void FTP_Client::initCommandList()
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
	int pos = command.find_first_of(' ');
	if (pos != string::npos) {
		command = command.substr(0, pos);
	}

	for (auto cmd : CommandList)
		if (command == cmd)
		{
			return true;
		}
	return false;
}

void FTP_Client::executeCommand(string command)
{
	Command cmd = getCommandValue(command);

	switch (cmd)
	{
	case LS:
	case DIR:
	case PUT:
	case GET:
		CommandHandler->portRelatedCommands(command);
		break;

	case MPUT:
		break;
	case MGET:
		break;
	case CD:
		CommandHandler->directoryCommands(command, "Remote directory:", 2, "CWD %s\r\n");
		break;
	case LCD:
		CommandHandler->lcd(command);
		break;
	case _DELETE:
		CommandHandler->directoryCommands(command, "Remote file: ", 4, "DELE %s\r\n");
		break;
	case MDELETE:
		break;
	case MKDIR:
		CommandHandler->directoryCommands(command, "Directory name: ", 5, "XMKD %s\r\n");
		break;
	case RMDIR:
		CommandHandler->directoryCommands(command, "Directory name: ", 5, "XRMD %s\r\n");
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