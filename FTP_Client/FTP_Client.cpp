// ftp_clnt_csocket.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "FTP_Client.h"

vector<pair<int, string>> ResponseErrorException::errorCodeList = {};

FTP_Client::FTP_Client()
{
}

FTP_Client::~FTP_Client()
{
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