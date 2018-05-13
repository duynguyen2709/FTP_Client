#include "stdafx.h"
#include "ResponseErrorException.h"

vector<pair<int, string>> ResponseErrorException::ErrorCodeList = {};

void ResponseErrorException::initErrorCodeList()
{
	ErrorCodeList.push_back(make_pair(0, "Not connected"));
	ErrorCodeList.push_back(make_pair(1, "Invalid command"));
	ErrorCodeList.push_back(make_pair(2, "Socket connection failed"));
	ErrorCodeList.push_back(make_pair(200, "Command okay"));
	ErrorCodeList.push_back(make_pair(202, "Command not implemented, superfluous at this site."));
	ErrorCodeList.push_back(make_pair(421, "Service not available, closing control connection."));
	ErrorCodeList.push_back(make_pair(500, "Syntax error, command unrecognized.This may include errors such as command line too long."));
	ErrorCodeList.push_back(make_pair(501, "Syntax error in parameters or arguments."));
	ErrorCodeList.push_back(make_pair(502, "Command not implemented."));
	ErrorCodeList.push_back(make_pair(503, "Bad sequence of commands."));
	ErrorCodeList.push_back(make_pair(530, "Login or password incorrect."));
	ErrorCodeList.push_back(make_pair(550, "Requested action not taken."));
}

void ResponseErrorException::setErrorCode(int code)
{
	errorCode = code;
}

const std::string ResponseErrorException::getErrorStringResponse() const throw()
{
	for (auto err : ErrorCodeList)
	{
		if (err.first == errorCode)
			return err.second;
	}
	return "";
}