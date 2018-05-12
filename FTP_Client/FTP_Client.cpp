#include "stdafx.h"
#include "FTP_Client.h"

//static fields re-declaration
vector<string> FTP_Client::CommandList = {};
My_IP_Address *FTP_Client::ipAddress = nullptr;

FTP_Client::FTP_Client()
{
	ConnectionStatus = false;
	Mode = _ACTIVE;
	CommandHandler = static_cast<IHandleCommand *>(&(*this));

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error: %ld\n", iResult);
	}
}

FTP_Client::~FTP_Client()
{
	ConnectionStatus = false;
	ClientSocket.Close();
}

bool FTP_Client::checkLegitIPAddress(const string command)
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
	{
		server = IP_Server;
		return true;
	}

	string IPFromHostName = resolveDomainToIP(IP_Server);

	if (IPFromHostName != "")
	{
		if (regex_match(IPFromHostName, ipAddressFormat))
		{
			server = IPFromHostName;
			return true;
		}
	}
	else cout << "Invalid IP/Host Address" << endl;

	return false;
}

string FTP_Client::resolveDomainToIP(const string host)
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

	//substring to get only the command
	int pos = command.find_first_of(' ');
	if (pos != NOT_FOUND) {
		command = command.substr(0, pos);
	}

	if (command == "ls")
	{
		cmd = LS;
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
		cmd = DIR;
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
		cmd = LCD;
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
		if (pos == NOT_FOUND)
			cmd = PWD;
		else {
			ex.setErrorCode(501);
			throw ex;
		}
	}
	else if (command == "passive")
	{
		if (pos == NOT_FOUND)
			cmd = PASSIVE;
		else {
			ex.setErrorCode(501);
			throw ex;
		}
	}

	return cmd;
}

bool FTP_Client::login(const string command)
{
	if (!checkLegitIPAddress(command))
		return false;

	ResponseErrorException ex;
	try
	{
		ClientSocket.Create();

		if (ClientSocket.Connect(LPCTSTR(CA2T(server.c_str())), 21) == 0)
		{
			ClientSocket.Close();
			return false;
		}

		char buf[BUFSIZ + 1];
		int tmpres, size;

		char * str;
		int codeftp;

		memset(buf, 0, sizeof buf);
		while ((tmpres = ClientSocket.Receive(buf, BUFSIZ, 0)) > 0) {
			sscanf(buf, "%d", &codeftp);
			cout << buf;

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

		if (server == "127.0.0.1") {
			ipAddress = new My_IP_Address(127, 0, 0, 1);
		}
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
	CommandList.push_back("active");
	CommandList.push_back("quit");
	CommandList.push_back("exit");
}

bool FTP_Client::checkCommand(string command)
{
	int pos = command.find_first_of(' ');
	if (pos != NOT_FOUND) {
		command = command.substr(0, pos);
	}

	for (auto cmd : CommandList)
		if (command == cmd)
		{
			return true;
		}
	return false;
}

void FTP_Client::executeCommand(const string command)
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

	case CD:
		CommandHandler->nonPortRelatedCommands(command, "Remote directory:", 2, "CWD %s\r\n");
		break;
	case LCD:
		CommandHandler->lcd(command);
		break;
	case _DELETE:
		CommandHandler->nonPortRelatedCommands(command, "Remote file: ", 6, "DELE %s\r\n");
		break;
	case MPUT:
	case MGET:
	case MDELETE:
		CommandHandler->multipleFilesCommands(command);
		break;
	case MKDIR:
		CommandHandler->nonPortRelatedCommands(command, "Directory name: ", 5, "XMKD %s\r\n");
		break;
	case RMDIR:
		CommandHandler->nonPortRelatedCommands(command, "Directory name: ", 5, "XRMD %s\r\n");
		break;
	case PWD:
		CommandHandler->pwd();
		break;

	case PASSIVE:
		Mode = _PASSIVE;
		cout << "Switched to PASSIVE Mode" << endl;
		break;

	case ACTIVE:
		Mode = _ACTIVE;
		cout << "Switched to ACTIVE Mode" << endl;
		break;

	default:
		break;
	}
	return;
}