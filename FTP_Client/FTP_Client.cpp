// ftp_clnt_csocket.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "FTP_Client.h"

vector<string> FTP_Client::CommandList = {};
vector<pair<int, string>> ResponseErrorException::ErrorCodeList = {};

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
	//
	//CHECK VALID IP ADDRESS
	//
	string IP_Server;
	if (command == "open")
	{
		cout << "To :";
		getline(cin, IP_Server);
	}
	else
		IP_Server = command.substr(5);

	regex ipAddressFormat("(^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])$)");

	if (!regex_match(IP_Server, ipAddressFormat))
	{
		cout << "Invalid IP Address" << endl;
		return false;
	}

	//END CHECK IP

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
	throw std::logic_error("The method or operation is not implemented.");
}

void ResponseErrorException::InitErrorCodeList()
{
	ErrorCodeList.push_back(make_pair(0, "Not connected"));
	ErrorCodeList.push_back(make_pair(1, "Invalid command"));
	ErrorCodeList.push_back(make_pair(200, "Command okay"));
	ErrorCodeList.push_back(make_pair(500, "Syntax error, command unrecognized.This may include errors such as command line too long."));
	ErrorCodeList.push_back(make_pair(501, "Syntax error in parameters or arguments."));
	ErrorCodeList.push_back(make_pair(202, "Command not implemented, superfluous at this site."));
	ErrorCodeList.push_back(make_pair(502, "Command not implemented."));
	ErrorCodeList.push_back(make_pair(503, "Bad sequence of commands."));
	ErrorCodeList.push_back(make_pair(530, "Not logged in."));
}