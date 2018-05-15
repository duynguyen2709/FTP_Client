#include "stdafx.h"
#include "FTP_Client.h"

vector<int> IHandleCommand::PortUsed = {};

SOCKET IHandleCommand::createDataSocket(int port)
{
	int iResult;
	SOCKET DataSocket;
	DataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (DataSocket == INVALID_SOCKET) {
		ex.setErrorCode(2);
		throw ex;
	}

	if (Mode == _ACTIVE)
	{
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = INADDR_ANY;
		service.sin_port = htons((u_short)port);

		if (::bind(DataSocket,
			(SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR) {
			closesocket(DataSocket);
			ex.setErrorCode(2);
			throw ex;
		}

		if (listen(DataSocket, 1) == SOCKET_ERROR) {
			closesocket(DataSocket);
			ex.setErrorCode(2);
			throw ex;
		}
	}
	else if (Mode == _PASSIVE) {
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = inet_addr(server.c_str());
		service.sin_port = htons((u_short)port);

		iResult = connect(DataSocket, (SOCKADDR *)& service, sizeof(service));
		if (iResult == SOCKET_ERROR) {
			iResult = closesocket(DataSocket);
			ex.setErrorCode(2);
			throw ex;
		}
	}

	return DataSocket;
}

int IHandleCommand::getNextFreePort() {
	int port;

	if (PortUsed.empty())
	{
		port = 52700 + rand() % 5000;
	}
	else
		port = PortUsed.back() + 1;

	PortUsed.push_back(port);
	return port;
}

int IHandleCommand::sendPORTCommand()
{
	char buf[BUFSIZ + 1];

	int resCode;

	int port = getNextFreePort();
	int temp = port / 256;

	sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", IHandleCommand::ipAddress->x1, IHandleCommand::ipAddress->x2, IHandleCommand::ipAddress->x3, IHandleCommand::ipAddress->x4, temp, port - temp * 256);

	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;

	int codeftp;
	sscanf(buf, "%d", &codeftp);
	if (codeftp != 200)
	{
		ex.setErrorCode(codeftp);
		throw ex;
	}

	return port;
}

int IHandleCommand::getPortInPassiveMode()
{
	int port = 0, resCode = 0;
	char buf[BUFSIZ + 1];
	sprintf(buf, "PASV\r\n");
	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);

	string find_226(buf);
	bool check_226 = false;
	if (find_226.find("226") != NOT_FOUND)
	{
		memset(buf, 0, sizeof buf);
		resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	}

	string temp(buf);

	int pos = 0;
	for (int i = 0; i < 4; i++)
		pos = temp.find(',', pos + 1);

	int pos2 = temp.find(',', pos + 1);
	port = stoi(temp.substr(pos + 1, pos2 - pos - 1)) * 256;
	pos = temp.find_first_of(')');
	port += stoi(temp.substr(pos2 + 1, pos - pos2 - 1));

	return port;
}

void IHandleCommand::checkSpaceInParameter(string param, string &srcFileName, string &dstFileName, int secondSpace)
{
	//one parameter contains spaces
	regex oneParam("^\".+\"$");

	//2 parameters & contain spaces
	regex twoParams_2Spaces("^\".+\" \".+\"$");

	//2 parameters & only 1 contains spaces
	regex twoParams_1Spaces_atSource("^\".+\" .+$");
	regex twoParams_1Spaces_atDest("^.+ \".+\"$");

	int pos, pos2;

	if (regex_match(param, twoParams_2Spaces))
	{
		pos = param.find('"', 1);
		pos2 = param.find('"', pos + 1);
		srcFileName = param.substr(1, pos - 1);
		dstFileName = param.substr(pos2 + 1, param.length() - pos2 - 2);
	}
	else if (regex_match(param, twoParams_1Spaces_atSource))
	{
		pos = param.find('"', 1);
		srcFileName = param.substr(1, pos - 1);
		dstFileName = param.substr(pos + 2);
	}
	else if (regex_match(param, twoParams_1Spaces_atDest))
	{
		pos = param.find_first_of(' ');
		srcFileName = param.substr(0, pos);
		dstFileName = param.substr(pos + 2, param.length() - pos - 3);
	}
	else if (regex_match(param, oneParam)) {
		srcFileName = dstFileName = param.substr(1, param.length() - 2);
	}
	else {
		if (secondSpace == NOT_FOUND)
			srcFileName = dstFileName = param;
		else {
			pos = param.find_first_of(' ');
			pos2 = param.find(' ', pos + 1);
			srcFileName = param.substr(0, pos);
			if (pos2 == NOT_FOUND)
				dstFileName = param.substr(pos + 1, param.length() - pos - 1);
			else
				dstFileName = param.substr(pos + 1, pos2 - pos - 1);
		}
	}
}

const char * IHandleCommand::formatBuffer(string command, string & srcFileName, string & dstFileName)
{
	//check if parameters' existence
	int pos = command.find_first_of(' ');
	int pos2 = command.find(' ', pos + 1);

	if (command.find("dir") != NOT_FOUND || command.find("ls") != NOT_FOUND)
	{
		if (pos != NOT_FOUND) {
			vector<string> fileType;
			getFileTypesFromParam(fileType, command.substr(pos + 1));
			if (fileType.size() > 1)
			{
				ex.setErrorCode(501);
				throw ex;
			}
			string temp = fileType[0];
			char buf[BUFSIZ];
			if (command.find("dir") != NOT_FOUND) {
				sprintf(buf, "LIST %s\r\n", temp.c_str());
			}
			else if (command.find("ls") != NOT_FOUND) {
				sprintf(buf, "NLST %s\r\n", temp.c_str());
			}
			return buf;
		}

		else {
			if (command.find("dir") != NOT_FOUND)
				return "LIST\r\n";

			else if (command.find("ls") != NOT_FOUND)
				return "NLST\r\n";
		}
	}

	//GET Command
	if (command.find("get") != NOT_FOUND) {

		//no parameter
		if (pos == NOT_FOUND) {
			cout << "Remote file ";
			getline(cin, srcFileName);

			cout << "Local file ";
			getline(cin, dstFileName);
		}
		else {
			string param = command.substr(pos + 1);
			checkSpaceInParameter(param, srcFileName, dstFileName, pos2);
		}

		char temp[BUFSIZ];
		sprintf(temp, "RETR %s\r\n", srcFileName.c_str());
		return temp;
	}

	//PUT Command
	if (command.find("put") != NOT_FOUND) {

		//no parameter
		if (pos == NOT_FOUND) {
			cout << "Local file ";
			getline(cin, srcFileName);

			cout << "Remote file ";
			getline(cin, dstFileName);
		}
		else {
			string param = command.substr(pos + 1);
			checkSpaceInParameter(param, srcFileName, dstFileName, pos2);
		}

		if (ifstream(srcFileName).good())
		{
			char temp[BUFSIZ];
			sprintf(temp, "STOR %s\r\n", dstFileName.c_str());
			return temp;
		}
		else {
			cout << srcFileName << ": File not found" << endl;
			return "";
		}
	}
}

void IHandleCommand::multipleFilesCommands(const string command)
{
	int pos = command.find_first_of(' ');

	vector<string> fileType, fileList;
	string param, cmd;
	char buf[BUFSIZ + 1];
	memset(buf, 0, sizeof buf);
	int resCode;

	if (pos == NOT_FOUND) {
		cmd = command;
		if (cmd == "mput")
			cout << "Local files ";
		else
			cout << "Remote files ";
		getline(cin, param);
	}
	else {
		param = command.substr(pos + 1);
		cmd = command.substr(0, pos);
	}
	getFileTypesFromParam(fileType, param);

	if (cmd == "mput") {
		for (auto f : fileType)
		{
			string dir(getcwd(buf, BUFSIZ));
			dir += "\\" + f;
			getFileListInCurrentDir(fileList, dir);
		}
	}
	else
	{
		for (auto fType : fileType) {

			// 			if (fType.find(' ', 0) != NOT_FOUND) {
			// 				fType = "\"" + fType + "\"";
			// 			}
			// 			string temp = "ls " + fType;
			// 			portRelatedCommands(temp);
			int port;

			if (Mode == _ACTIVE)
			{
				port = sendPORTCommand();
			}
			else if (Mode == _PASSIVE)
			{
				port = getPortInPassiveMode();
			}

			SOCKET DataSocket = createDataSocket(port);

			memset(buf, 0, sizeof buf);
			sprintf(buf, "NLST %s\r\n", fType.c_str());
			resCode = ClientSocket.Send(buf, strlen(buf), 0);

			memset(buf, 0, sizeof buf);
			resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
			cout << buf;

			int codeftp;
			sscanf(buf, "%d", &codeftp);
			if (codeftp != 150)
			{
				closesocket(DataSocket);
				ex.setErrorCode(codeftp);
				throw ex;
			}

			int iResult;

			if (Mode == _ACTIVE)
			{
				SOCKET AcceptSocket;
				AcceptSocket = accept(DataSocket, NULL, NULL);

				memset(buf, 0, sizeof buf);
				resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
				cout << buf;
				memset(buf, 0, sizeof buf);

				if (AcceptSocket == INVALID_SOCKET) {
					closesocket(AcceptSocket);
					ex.setErrorCode(2);
					throw ex;
				}
				else
				{
					while ((iResult = recv(AcceptSocket, buf, BUFSIZ, 0)) > 0) {
						getFileListFromBuffer(fileList, buf);
						memset(buf, 0, iResult);
					}
				}
				closesocket(AcceptSocket);
			}
			else if (Mode == _PASSIVE)
			{
				memset(buf, 0, sizeof buf);

				while ((iResult = recv(DataSocket, buf, BUFSIZ, 0)) > 0) {
					getFileListFromBuffer(fileList, buf);
					memset(buf, 0, iResult);
				}
			}

			closesocket(DataSocket);

			memset(buf, 0, sizeof buf);
		}
	}

	for (auto f : fileList) {
		memset(buf, 0, sizeof buf);
		cout << cmd << " " << f << "?";

		if (f.find(' ', 0) != NOT_FOUND) {
			f = "\"" + f + "\"";
		}

		char c;
		cin >> c;
		if (tolower(c) == 'y') {
			if (cmd == "mdelete")
			{
				string str = "delete " + f;
				nonPortRelatedCommands(str, "Remote file: ", 6, "DELE %s\r\n");
			}
			else if (cmd == "mget")
			{
				string str = "get " + f;
				portRelatedCommands(str);
			}
			else if (cmd == "mput") {
				string str = "put " + f;
				portRelatedCommands(str);
			}
		}
	}
	cin.ignore();
	cin.clear();
}

void IHandleCommand::portRelatedCommands(string command)
{
	char buf[BUFSIZ + 1];

	int resCode;

	int port = 0;
	if (Mode == _ACTIVE) {
		port = sendPORTCommand();
	}
	else if (Mode == _PASSIVE) {
		port = getPortInPassiveMode();
	}

	SOCKET DataSocket = createDataSocket(port);

	string srcFileName, dstFileName;

	strcpy(buf, formatBuffer(command, srcFileName, dstFileName));
	if (strcmp(buf, "") == 0)
		return;

	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;

	int iResult;
	int codeftp = 0;

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 150)
	{
		closesocket(DataSocket);
		ex.setErrorCode(codeftp);
		throw ex;
	}

	string temp(buf);
	bool check_226 = false;
	if (temp.find("226") != NOT_FOUND)
		check_226 = true;

	memset(buf, 0, sizeof buf);

	if (Mode == _PASSIVE) {
		if (command.find("dir") != NOT_FOUND || command.find("ls") != NOT_FOUND) {
			while ((iResult = recv(DataSocket, buf, BUFSIZ, 0)) > 0) {
				cout << buf;
				memset(buf, 0, iResult);
			}
		}
		else if (command.find("get") != NOT_FOUND)
			get(DataSocket, dstFileName);
		else if (command.find("put") != NOT_FOUND)
			put(DataSocket, srcFileName);
	}
	else if (Mode == _ACTIVE)
	{
		SOCKET AcceptSocket;
		AcceptSocket = accept(DataSocket, NULL, NULL);

		if (AcceptSocket == INVALID_SOCKET) {
			closesocket(DataSocket);
			ex.setErrorCode(2);
			throw ex;
		}
		else
		{
			//DIR/LS Command

			if (command.find("dir") != NOT_FOUND || command.find("ls") != NOT_FOUND) {
				while ((iResult = recv(AcceptSocket, buf, BUFSIZ, 0)) > 0) {
					cout << buf;
					memset(buf, 0, iResult);
				}
			}

			//GET Command
			else if (command.find("get") != NOT_FOUND)
				get(AcceptSocket, dstFileName);

			//PUT Command
			else if (command.find("put") != NOT_FOUND)
				put(AcceptSocket, srcFileName);
		}

		closesocket(AcceptSocket);
	}

	if ((iResult = closesocket(DataSocket)) == SOCKET_ERROR) {
		cout << "Close socket error :" << WSAGetLastError() << endl;
		ex.setErrorCode(2);
		throw ex;
	}

	if (check_226 == false)
	{
		memset(buf, 0, sizeof buf);
		resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
		cout << buf;
	}
}

void IHandleCommand::get(SOCKET AcceptSocket, string dstFileName)
{
	int iResult;
	char buf[BUFSIZ + 1];

	ofstream outputFile;
	outputFile.open(dstFileName.c_str(), ios::out | ios::binary | ios::trunc);

	while ((iResult = recv(AcceptSocket, buf, BUFSIZ, 0)) > 0) {
		outputFile.write(buf, iResult);
		memset(buf, 0, iResult);
	}

	outputFile.close();
}

void IHandleCommand::put(SOCKET AcceptSocket, string srcFileName)
{
	char buf[BUFSIZ + 1];

	ifstream inputFile;
	inputFile.open(srcFileName.c_str(), ios::binary | ios::in);

	//get file's size
	inputFile.seekg(0, ios_base::end);
	int length = inputFile.tellg();
	inputFile.seekg(0, ios_base::beg);

	//load file to buffer and send
	while (length > BUFSIZ)
	{
		inputFile.read(buf, BUFSIZ);
		send(AcceptSocket, buf, BUFSIZ, 0);
		length -= BUFSIZ;
	}

	inputFile.read(buf, length);
	send(AcceptSocket, buf, length, 0);

	inputFile.close();
}

void IHandleCommand::lcd(string command)
{
	char buf[BUFSIZ];

	//If there's no argument,
	//Then set local directory to user default home directory
	if (command.find_first_of(' ') == NOT_FOUND) {
		_chdir(getenv("USERPROFILE"));
		cout << "Local directory now " << getcwd(buf, BUFSIZ) << endl;
		return;
	}

	//
	//else
	//check if directory exists
	//then change to new directory
	//else print not found directory
	//
	string newDir = command.substr(4);

	wstring stemp = wstring(newDir.begin(), newDir.end());
	LPCWSTR dir = stemp.c_str();

	DWORD dwAttrib = GetFileAttributes(dir);

	if (!(dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))) {
		cout << newDir << ": File not found" << endl;
		return;
	}

	if (_chdir(newDir.c_str())) {

		//handling error
		switch (errno)
		{
		case ENOENT:
			cout << "Unable to locate the directory: " << newDir << endl;
			break;
		case EINVAL:
			cout << "Invalid buffer." << endl;
			break;
		default:
			cout << "Unknown error." << endl;
		}
		return;
	}

	cout << "Local directory now " << getcwd(buf, BUFSIZ) << endl;
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

void IHandleCommand::nonPortRelatedCommands(string command, string noti, const int commandLength, const char * format)
{
	char buf[BUFSIZ + 1];

	string dir;

	int resCode = 0;

	int space = command.find_first_of(' ');

	//if no argument then ask for directory name
	if (space == NOT_FOUND) {
		cout << noti;
		getline(cin, dir);
	}
	else {
		dir = command.substr(commandLength + 1);
	}

	//check if directory's name included space
	regex spaceIncluded("^\".+\"$");
	if (regex_match(dir, spaceIncluded)) {
		dir = dir.substr(1, dir.length() - 2);
	}
	else {
		space = dir.find_first_of(' ');
		if (space != NOT_FOUND) {
			dir = dir.substr(0, space);
		}
	}

	//send command & receive response
	sprintf(buf, format, dir.c_str());
	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);

	cout << buf;
}

void IHandleCommand::getFileListFromBuffer(vector<string> &fileList, const char * buf) {
	string str(buf);

	int firstPos = 0;
	int secondPos = str.find("\r\n", 0);

	while (secondPos < str.length() || secondPos != NOT_FOUND) {
		fileList.push_back(str.substr(firstPos, secondPos - firstPos));

		firstPos = secondPos + 2;
		secondPos = str.find("\r\n", firstPos);
	}
}

void IHandleCommand::getFileTypesFromParam(vector<string>& fileTypes, string param)
{
	if (std::count(param.begin(), param.end(), '"') % 2 == 1) {
		ex.setErrorCode(501);
		throw ex;
	}

	int pos = 0;
	string temp;
	while (param.length() > 0) {
		if (param[0] == '"') {
			pos = param.find('"', 1);
			temp = param.substr(1, pos - 1);
			pos++;
		}
		else {
			pos = param.find(' ', 1);
			if (pos == NOT_FOUND)
				pos = param.length();
			temp = param.substr(0, pos);
		}
		fileTypes.push_back(temp);
		param.erase(0, pos + 1);
	}
}

void IHandleCommand::getFileListInCurrentDir(vector<string> &fileList, const string dir) {
	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;

	szDir[dir.size()] = 0;
	std::copy(dir.begin(), dir.end(), szDir);

	hFind = FindFirstFile(szDir, &ffd);

	if (hFind == INVALID_HANDLE_VALUE) {
		ex.setErrorCode(3);
		throw ex;
	}
	do
	{
		char ch[260];
		WideCharToMultiByte(CP_UTF8, 0, ffd.cFileName, -1, ch, 260, NULL, NULL);
		string ss(ch);
		fileList.push_back(ss);
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}