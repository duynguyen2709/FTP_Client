// ftp_clnt_csocket.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "FTP_Client.h"

vector<pair<int, string>> ResponseErrorException::errorCodeList = {};

FTP_Client::FTP_Client()
{
	ResponseErrorException ex;

	try
	{
		// Tao socket dau tien
		ClientSocket.Create();

		// Ket noi den Server
		if (ClientSocket.Connect(_T(server), 21) != 0)
		{
			cout << "Ket noi toi Server thanh cong !!!" << endl << endl;
		}
		else
			return;

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
	}
	catch (ResponseErrorException &e)
	{
		cout << e.getErrorStringResponse() << endl;
	}
}

FTP_Client::~FTP_Client()
{
	ClientSocket.Close();
}

void ResponseErrorException::InitErrorCodeList()
{
	errorCodeList.push_back(make_pair(200, "Command okay"));
	errorCodeList.push_back(make_pair(500, "Syntax error, command unrecognized.This may include errors such as command line too long."));
	errorCodeList.push_back(make_pair(501, "Syntax error in parameters or arguments."));
	errorCodeList.push_back(make_pair(202, "Command not implemented, superfluous at this site."));
	errorCodeList.push_back(make_pair(502, "Command not implemented."));
	errorCodeList.push_back(make_pair(503, "Bad sequence of commands."));
	errorCodeList.push_back(make_pair(530, "Not logged in."));
}