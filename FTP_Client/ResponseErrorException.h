#include "stdafx.h"
#pragma once

class ResponseErrorException : public exception
{
public:

	ResponseErrorException() :errorCode(0) {};
	~ResponseErrorException() {};

	void setErrorCode(int code);
	const string getErrorStringResponse() const throw();

	static void initErrorCodeList();

private:
	static vector<pair<int, string>> ErrorCodeList;
	int errorCode;
};