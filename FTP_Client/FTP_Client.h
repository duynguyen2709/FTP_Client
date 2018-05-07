#pragma once

#include "resource.h"
#include "stdafx.h"
#include "ResponseErrorException.h"
#pragma comment(lib, "Ws2_32.lib")

#define server "103.207.36.66"

struct My_IP_Address {
	unsigned int x1;
	unsigned int x2;
	unsigned int x3;
	unsigned int x4;

	My_IP_Address() {
		HINTERNET hInternet, hFile;
		DWORD rSize = 0;
		char buffer[47];

		hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		hFile = InternetOpenUrlA(hInternet, "https://myexternalip.com/raw", NULL, 0, INTERNET_FLAG_RELOAD, 0);

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
	EXIT,
	_NULL
};

class IHandleCommand;

class FTP_Client
{
protected:

	CSocket ClientSocket;

private:

	bool ConnectionStatus;

	bool checkLegitIPAddress(const string command);

	string resolveDomainToIP(const string host);

	Command getCommandValue(string command);

	IHandleCommand *CommandHandler;
public:

	FTP_Client();

	~FTP_Client();

	bool isConnected() { return ConnectionStatus; }

	bool login(const string command);

	static vector<string> CommandList;

	static void initCommandList();

	static bool checkCommand(string command);

	void executeCommand(const string command);
};

//INTERFACE FOR HANDLING COMMANDS
class IHandleCommand : public FTP_Client {
private:
	ResponseErrorException ex;

	SOCKET createListeningSocket(const int port);

	static vector<int> PortUsed;

	int getNextFreePort() {
		int port;

		if (PortUsed.empty())
			port = 52700 + rand() % 2000;
		else
			port = PortUsed.back() + 1;

		PortUsed.push_back(port);
		return port;
	}

	const char* formatBuffer(const string command, string &srcFileName, string &dstFileName);

	int portCommand();

	void get(SOCKET AcceptSocket, const string dstFileName);
	void put(SOCKET AcceptSocket, const string srcFileName);

	void getFileListFromBuffer(vector<string> &fileList, const char * buf);
public:

	IHandleCommand() {};
	~IHandleCommand() {};

	static My_IP_Address *ipAddress;

	void mdelete(const string command);
	void portRelatedCommands(const string command);
	void lcd(const string command);
	void pwd();
	void oneArgCommands(const string command, const string noti, const int commandLength, const char * format);
};