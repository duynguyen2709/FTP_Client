// ftp_clnt_csocket.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "FTP_Client.h"

vector<string> FTP_Client::CommandList = {};
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
	else if (command.find("dir") != string::npos)
	{
		cmd = DIR;
	}
	else if (command.find("put") != string::npos)
	{
		cmd = PUT;
	}
	else if (command.find("get") != string::npos)
	{
		cmd = GET;
	}
	else if (command.find("mput") != string::npos)
	{
		cmd = MPUT;
	}
	else if (command.find("mget") != string::npos)
	{
		cmd = MGET;
	}
	else if (command.find("cd") != string::npos)
	{
		cmd = CD;
	}
	else if (command.find("lcd") != string::npos)
	{
		cmd = LCD;
	}
	else if (command.find("delete") != string::npos)
	{
		cmd = _DELETE;
	}
	else if (command.find("mdelete") != string::npos)
	{
		cmd = MDELETE;
	}
	else if (command.find("mkdir") != string::npos)
	{
		cmd = MKDIR;
	}
	else if (command.find("rmdir") != string::npos)
	{
		cmd = RMDIR;
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
		int tmpres, size, status;
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
		cout << "Connection established, waiting for welcome message...\n";

		//How to know the end of welcome message: http://stackoverflow.com/questions/13082538/how-to-know-the-end-of-ftp-welcome-message
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
	ifstream cin("CommandList.txt");
	string cmd;

	while (!cin.eof())
	{
		cin >> cmd;
		CommandList.push_back(cmd);
	}
	cin.clear();
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
	/*char buf[BUFSIZ + 1];
	int tmpres;

	My_IP_Address ipAddress;

	sprintf(buf, "PORT %d,%d,%d,%d,205,220\r\n", ipAddress.x1, ipAddress.x2, ipAddress.x3, ipAddress.x4);
	tmpres = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	tmpres = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf << endl;

	strcpy(buf, "LIST\r\n");
	tmpres = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	tmpres = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf << endl;*/

	Command cmd = commandValue(command);

	IHandleCommand *commandHandler = static_cast<IHandleCommand *>(&(*this));

	switch (cmd)
	{
	case LS:
		break;
	case DIR:
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
		commandHandler->cd(command);
		break;
	case LCD:
		break;
	case _DELETE:
		break;
	case MDELETE:
		break;
	case MKDIR:
		break;
	case RMDIR:
		break;
	case PWD:
		commandHandler->pwd();
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
	ifstream cin("ErrorCodeList.txt");
	int code;
	string err;

	while (!cin.eof())
	{
		cin >> code;
		getline(cin, err);
		ErrorCodeList.push_back(make_pair(code, err));
	}

	cin.clear();
}

void IHandleCommand::cd(string command)
{
	char buf[BUFSIZ + 1];
	char dir[BUFSIZ];
	int resCode = 0;

	if (command.find_first_of(' ') == string::npos) {
		cout << "Remote directory :";
		gets_s(dir, 255);
	}
	else {
		string temp = command.substr(3);
		strcpy(dir, temp.c_str());
	}

	sprintf(buf, "CWD %s\r\n", dir);
	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);

	cout << buf;
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