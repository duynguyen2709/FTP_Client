#pragma once

#include "resource.h"
#include "stdafx.h"

#define server "103.207.36.66"

class FTP_Client
{
private:
	CSocket client;

public:
	FTP_Client();
	~FTP_Client();
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
		for (auto err : errorCodeList)
		{
			if (err.first == errorCode)
				return err.second;
		}
		return "";
	}

private:
	static vector<pair<int, string>> errorCodeList;

	int errorCode;
};
