#include "stdafx.h"
#include "FTP_Client.h"

vector<int> IHandleCommand::PortUsed = {};

SOCKET IHandleCommand::createListeningSocket(int port)
{
	int iResult;
	SOCKET ListenSocket;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
		wprintf(L"Created Listening Socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return NULL;
	}

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons((u_short)port);

	if (::bind(ListenSocket,
		(SOCKADDR *)& service, sizeof(service)) == SOCKET_ERROR) {
		wprintf(L"Bind failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return NULL;
	}

	if (listen(ListenSocket, 1) == SOCKET_ERROR) {
		wprintf(L"Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return NULL;
	}

	return ListenSocket;
}

int IHandleCommand::getNextFreePort() {
	int port;

	if (PortUsed.empty())
		port = 52700 + rand() % 2000;
	else
		port = PortUsed.back() + 1;

	PortUsed.push_back(port);
	return port;
}

int IHandleCommand::portCommand()
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
	if (command == "dir")
		return "LIST\r\n";

	if (command == "ls")
		return "NLST\r\n";

	//check if parameters' existence
	int pos = command.find_first_of(' ');
	int pos2 = command.find(' ', pos + 1);

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

void IHandleCommand::mdelete(const string command)
{
	int pos = command.find_first_of(' ');

	string fileType;
	if (pos == NOT_FOUND) {
		cout << "Remote files";
		getline(cin, fileType);
	}
	else {
		fileType = command.substr(pos + 1);
	}

	char buf[BUFSIZ + 1];

	int resCode;

	vector<string> fileList;

	int port = portCommand();

	SOCKET ListenSocket = createListeningSocket(port);

	sprintf(buf, "NLST %s\r\n", fileType.c_str());
	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;
	memset(buf, 0, sizeof buf);

	SOCKET AcceptSocket;
	AcceptSocket = accept(ListenSocket, NULL, NULL);

	if (AcceptSocket == INVALID_SOCKET) {
		wprintf(L"Accept failed with error: %ld\n", WSAGetLastError());
	}
	else
	{
		int iResult;

		while ((iResult = recv(AcceptSocket, buf, BUFSIZ, 0)) > 0) {
			getFileListFromBuffer(fileList, buf);

			memset(buf, 0, iResult);
		}
	}
	for (auto f : fileList) {
		cout << "mdelete " << f << "?";
		char c;
		cin >> c;
		if (tolower(c) == 'y') {
			string str = "delete " + f;
			oneArgCommands(str, "Remote file: ", 6, "DELE %s\r\n");
		}
	}

	cin.ignore();
	cin.clear();

	closesocket(ListenSocket);
	closesocket(AcceptSocket);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;
}

void IHandleCommand::portRelatedCommands(string command)
{
	char buf[BUFSIZ + 1];

	int resCode;

	int port = portCommand();

	SOCKET ListenSocket = createListeningSocket(port);

	string srcFileName, dstFileName;

	strcpy_s(buf, formatBuffer(command, srcFileName, dstFileName));
	if (strcmp(buf, "") == 0)
		return;

	resCode = ClientSocket.Send(buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;

	SOCKET AcceptSocket;
	AcceptSocket = accept(ListenSocket, NULL, NULL);

	if (AcceptSocket == INVALID_SOCKET) {
		wprintf(L"Accept failed with error: %ld\n", WSAGetLastError());
	}
	else
	{
		int iResult;

		//DIR/LS Command
		if (command == "dir" || command == "ls") {
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

	closesocket(ListenSocket);
	closesocket(AcceptSocket);

	memset(buf, 0, sizeof buf);
	resCode = ClientSocket.Receive(buf, BUFSIZ, 0);
	cout << buf;
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

void IHandleCommand::oneArgCommands(string command, string noti, const int commandLength, const char * format)
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