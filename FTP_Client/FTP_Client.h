#pragma once

#include "resource.h"
#include "stdafx.h"

#define server "103.207.36.66"

class FTP_Client
{
private:
	CSocket ClientSocket;
	bool ConnectionStatus;

	bool isLegitIPAddress(string command);

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
