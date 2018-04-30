#pragma once

#include "resource.h"
#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#include <corecrt_io.h>

#define server "103.207.36.66"

struct My_IP_Address {
	unsigned int x1;
	unsigned int x2;
	unsigned int x3;
	unsigned int x4;

	My_IP_Address() {
		HINTERNET hInternet, hFile;
		DWORD rSize;
		char buffer[47];

		hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		hFile = InternetOpenUrlA(hInternet, "http://www.passwordrandom.com/query?command=ip", NULL, 0, INTERNET_FLAG_RELOAD, 0);
		InternetReadFile(hFile, &buffer, sizeof(buffer), &rSize);
		buffer[rSize] = '\0';

		InternetCloseHandle(hFile);
		InternetCloseHandle(hInternet);

		string str(buffer);

		int pos = str.find_first_of('.');
		int pos2 = str.find('.', pos + 1);
		int pos3 = str.find_last_of('.');

		x1 = stoi(str.substr(0, pos));
		x2 = stoi(str.substr(pos + 1, pos2 - pos - 1));
		x3 = stoi(str.substr(pos2 + 1, pos3 - pos2 - 1));
		x4 = stoi(str.substr(pos3 + 1));
	}
};

enum Command {
	LS,
	DIR,
	PUT,
	GET,
	MPUT,
	MGET,
	CD,
	LCD,
	_DELETE,
	MDELETE,
	MKDIR,
	RMDIR,
	PWD,
	PASSIVE,
	QUIT,
	EXIT
};

class IHandleCommand;

class FTP_Client
{
protected:

	CSocket ClientSocket;

private:

	bool ConnectionStatus;

	bool isLegitIPAddress(string command);

	Command commandValue(string command);

	IHandleCommand *CommandHandler;

public:

	FTP_Client();

	~FTP_Client();

	bool isConnected() { return ConnectionStatus; }

	bool Login(string command);

	static vector<string> CommandList;

	static void InitCommandList();

	static bool checkCommand(string command);

	void ExecuteCommand(string command);
};

class ResponseErrorException : public exception
{
public:
	static void InitErrorCodeList();

	ResponseErrorException() :errorCode(0) {};

	~ResponseErrorException() {};

	void setErrorCode(int code) {
		errorCode = code;
	}

	const string getErrorStringResponse() const throw() {
		for (auto err : ErrorCodeList)
		{
			if (err.first == errorCode)
				return err.second;
		}
		return "";
	}

private:
	static vector<pair<int, string>> ErrorCodeList;

	int errorCode;
};

//INTERFACE FOR HANDLING COMMANDS
class IHandleCommand : public FTP_Client {
private:
	ResponseErrorException ex;

	SOCKET createListeningSocket(int port);

	static vector<int> PortUsed;

	int randomPort() {
		int port;
	randomPort: port = 52700 + rand() % 2000;

		for (int i = 0; i < PortUsed.size(); i++)
		{
			if (PortUsed[i] == port)
				goto randomPort;
		}

		PortUsed.push_back(port);
		return port;
	}

	int portCommand();

public:

	IHandleCommand() { };

	~IHandleCommand() {	};

	void dir(string command);
	void lcd(string command);
	void pwd();
	void serverSideCommands(string command, string noti, const int commandLength, const char * format);
};