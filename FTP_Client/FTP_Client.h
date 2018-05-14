#pragma once

#include "resource.h"
#include "stdafx.h"
#include "ResponseErrorException.h"

struct My_IP_Address {
	unsigned int x1 = 0;
	unsigned int x2 = 0;
	unsigned int x3 = 0;
	unsigned int x4 = 0;

	//************************************
	// Method:    My_IP_Address
	// FullName:  My_IP_Address::My_IP_Address
	// Access:    public
	// Returns:   IP Address in Struct
	// Qualifier: Get External IP Address
	//************************************
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

		if (str != "")
		{
			int pos = str.find_first_of('.');
			int pos2 = str.find('.', pos + 1);
			int pos3 = str.find_last_of('.');

			x1 = stoi(str.substr(0, pos));
			x2 = stoi(str.substr(pos + 1, pos2 - pos - 1));
			x3 = stoi(str.substr(pos2 + 1, pos3 - pos2 - 1));
			x4 = stoi(str.substr(pos3 + 1));
		}
	}

	//************************************
	// Method:    My_IP_Address
	// FullName:  My_IP_Address::My_IP_Address
	// Access:    public
	// Returns:
	// Qualifier: Set Own IP Address from parameters
	// Parameter: int h1
	// Parameter: int h2
	// Parameter: int h3
	// Parameter: int h4
	//************************************
	My_IP_Address(int h1, int h2, int h3, int h4) {
		x1 = h1;
		x2 = h2;
		x3 = h3;
		x4 = h4;
	}
};

//************************************
// Enum:		Command
// Access:		public
// Data:		Contains Available FTP Commands
//************************************
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
	ACTIVE,
	QUIT,
	EXIT,
	_NULL
};

//************************************
// Enum:		ConnectionMode
// Access:		public
// Data:		Contains 2 Connection Modes in FTP
//************************************
enum ConnectionMode {
	_ACTIVE,
	_PASSIVE
};

class IHandleCommand;

//************************************
// Class:		FTP_Client
// Access:		public
// Using:		Main Class for each client
//************************************
class FTP_Client
{
protected:

	//************************************
	// Field:			ClientSocket
	// Access:			protected
	// Description:		Main Socket for connection to FTP Server
	//************************************
	CSocket ClientSocket;

	//************************************
	// Field:			Mode
	// Data type:		ConnectionMode
	// Access:			protected
	// Description:		Current Connecting Mode
	//************************************
	ConnectionMode Mode;

	//************************************
	// Field:			server
	// Data type:		string
	// Access:			protected
	// Description:		The Server which client is currently connecting to
	//************************************
	string server = "127.0.0.1";

private:

	//************************************
	// Field:			ConnectionStatus
	// Data Type:		bool
	// Access:			private
	// Description:		Connection status (Logon or Not)
	//************************************
	bool ConnectionStatus;

	//************************************
	// Field:			CommandHandler
	// Data Type:		IHandleCommand
	// Access:			private
	// Description:		Main Handler to execute commands
	//************************************
	IHandleCommand *CommandHandler;

	//************************************
	// Method:    setOwnIP
	// FullName:  FTP_Client::setOwnIP
	// Access:    private
	// Returns:   void
	// Qualifier: Set Own IP Address based on server (localhost/LAN/Internet)
	//************************************
	void setOwnIP();

	//************************************
	// Method:    checkLegitIPAddress
	// FullName:  FTP_Client::checkLegitIPAddress
	// Access:    private
	// Returns:   bool
	// Qualifier: check whether IP Address entered is legit or not
	// Parameter: const string command
	//************************************
	bool checkLegitIPAddress(const string command);

	//************************************
	// Method:    resolveDomainToIP
	// FullName:  FTP_Client::resolveDomainToIP
	// Access:    private
	// Returns:   std::string
	// Qualifier: Convert the hostname entered to IP Address (DNS Resolve)
	// Parameter: const string host
	//************************************
	string resolveDomainToIP(const string host);

	//************************************
	// Method:    getCommandValue
	// FullName:  FTP_Client::getCommandValue
	// Access:    private
	// Returns:   Command
	// Qualifier: return value of commands in struct
	// Parameter: string command
	//************************************
	Command getCommandValue(string command);

public:

	//************************************
	// Field:			CommandList
	// Data Type:		static vector<string>
	// Access:			public
	// Description:		Contains commands list in string type
	//************************************
	static vector<string> CommandList;

	//************************************
	// Field:			ipAddress
	// Data Type:		static My_IP_Address
	// Access:			public
	// Description:		Contains client's ip address
	//************************************
	static My_IP_Address *ipAddress;

	//************************************
	// Method:    FTP_Client
	// FullName:  FTP_Client::FTP_Client
	// Access:    public
	// Qualifier: Default Constructor, Init Handler,wsaData
	//************************************
	FTP_Client();

	//************************************
	// Method:    ~FTP_Client
	// FullName:  FTP_Client::~FTP_Client
	// Access:    public
	// Qualifier: Default Destructor, Close Main Socket
	//************************************
	~FTP_Client();

	//************************************
	// Method:    isConnected
	// FullName:  FTP_Client::isConnected
	// Access:    public
	// Returns:   bool
	// Qualifier: check whether this client is connected to server or not
	//************************************
	bool isConnected() { return ConnectionStatus; }

	//************************************
	// Method:    login
	// FullName:  FTP_Client::login
	// Access:    public
	// Returns:   bool
	// Qualifier: Login to FTP Server, return login successed or not
	// Parameter: const string command
	//************************************
	bool login(const string command);

	//************************************
	// Method:    initCommandList
	// FullName:  FTP_Client::initCommandList
	// Access:    public static
	// Returns:   void
	// Qualifier: Initialize Commands List
	//************************************
	static void initCommandList();

	//************************************
	// Method:    checkCommand
	// FullName:  FTP_Client::checkCommand
	// Access:    public static
	// Returns:   bool
	// Qualifier: check whether command entered is legit or not
	// Parameter: string command
	//************************************
	static bool checkCommand(string command);

	//************************************
	// Method:    executeCommand
	// FullName:  FTP_Client::executeCommand
	// Access:    public
	// Returns:   void
	// Qualifier: execute the command entered
	// Parameter: const string command
	//************************************
	void executeCommand(const string command);
};

//************************************
// Class:		IHandleCommand
// Access:		public
// Using:		Class Acts as an Interfaces for handling commands
//************************************
class IHandleCommand : public FTP_Client {
private:

	//************************************
	// Field:			ex
	// Data type:		ResponseErrorException
	// Access:			private
	// Description:		Exception to be thrown when errors occur
	//************************************
	ResponseErrorException ex;

	//************************************
	// Field:			PortUsed
	// Data type:		static vector<int>
	// Access:			private
	// Description:		List of Port used for PORT Command
	//************************************
	static vector<int> PortUsed;

	//************************************
	// Method:    createDataSocket
	// FullName:  IHandleCommand::createListeningSocket
	// Access:    private
	// Returns:   SOCKET
	// Qualifier: create a socket for transfering data
	// Parameter: const int port
	//************************************
	SOCKET createDataSocket(const int port);

	//************************************
	// Method:    getNextFreePort
	// FullName:  IHandleCommand::getNextFreePort
	// Access:    private
	// Returns:   int
	// Qualifier: Get next free port for next PORT Command
	//************************************
	int getNextFreePort();

	//************************************
	// Method:    sendPORTCommand
	// FullName:  IHandleCommand::portCommand
	// Access:    private
	// Returns:   int : port used in command
	// Qualifier: execute PORT Command
	//************************************
	int sendPORTCommand();

	//************************************
	// Method:    getPortInPassiveMode
	// FullName:  IHandleCommand::getPortInPassiveMode
	// Access:    private
	// Returns:   int
	// Qualifier: Send PASV Command & Get Port in response
	//************************************
	int getPortInPassiveMode();

	//************************************
	// Method:    checkSpaceInParameter
	// FullName:  IHandleCommand::checkSpaceInParameter
	// Access:    private
	// Returns:   void
	// Qualifier: check if parameters contain spaces, get filenames based on parameters
	// Parameter: string param
	// Parameter: string & srcFileName
	// Parameter: string & dstFileName
	// Parameter: int secondSpace
	//************************************
	void checkSpaceInParameter(string param, string &srcFileName, string &dstFileName, int secondSpace);

	//************************************
	// Method:    formatBuffer
	// FullName:  IHandleCommand::formatBuffer
	// Access:    private
	// Returns:   const char*
	// Qualifier: format buffer before sending for 4 commands :DIR,LS,GET,PUT
	// Parameter: const string command
	// Parameter: string & srcFileName
	// Parameter: string & dstFileName
	//************************************
	const char* formatBuffer(const string command, string &srcFileName, string &dstFileName);

	//************************************
	// Method:    get
	// FullName:  IHandleCommand::get
	// Access:    private
	// Returns:   void
	// Qualifier: GET Command
	// Parameter: SOCKET AcceptSocket
	// Parameter: const string dstFileName
	//************************************
	void get(SOCKET AcceptSocket, const string dstFileName);

	//************************************
	// Method:    put
	// FullName:  IHandleCommand::put
	// Access:    private
	// Returns:   void
	// Qualifier: PUT Command
	// Parameter: SOCKET AcceptSocket
	// Parameter: const string srcFileName
	//************************************
	void put(SOCKET AcceptSocket, const string srcFileName);

	//************************************
	// Method:    getFileListFromBuffer
	// FullName:  IHandleCommand::getFileListFromBuffer
	// Access:    private
	// Returns:   void
	// Qualifier: Parse buffer to get file list
	// Parameter: vector<string> & fileList
	// Parameter: const char * buf
	//************************************
	void getFileListFromBuffer(vector<string> &fileList, const char * buf);

	//************************************
	// Method:    getFileTypesFromParam
	// FullName:  IHandleCommand::getFileTypesFromParam
	// Access:    private
	// Returns:   void
	// Qualifier: Parse Parameters to get file types to send in NLST Command
	// Parameter: vector<string> & fileTypes
	// Parameter: string param
	//************************************
	void getFileTypesFromParam(vector<string> &fileTypes, string param);

	//************************************
	// Method:    getFileListInCurrentDir
	// FullName:  IHandleCommand::getFileListInCurrentDir
	// Access:    private
	// Returns:   void
	// Qualifier: Get files fit condition in current directory
	// Parameter: vector<string> & fileList
	// Parameter: const string dir
	//************************************
	void getFileListInCurrentDir(vector<string> &fileList, const string dir);

public:

	//************************************
	// Method:    IHandleCommand
	// FullName:  IHandleCommand::IHandleCommand
	// Access:    public
	// Returns:
	// Qualifier: Default Constructor
	//************************************
	IHandleCommand() {};

	//************************************
	// Method:    ~IHandleCommand
	// FullName:  IHandleCommand::~IHandleCommand
	// Access:    public
	// Returns:
	// Qualifier: Default Desstructor
	//************************************
	~IHandleCommand() {};

	void multipleFilesCommands(const string command);

	//************************************
	// Method:    portRelatedCommands
	// FullName:  IHandleCommand::portRelatedCommands
	// Access:    public
	// Returns:   void
	// Qualifier: execute commands which consist PORT Command (PUT,GET,DIR,LS...)
	// Parameter: const string command
	//************************************
	void portRelatedCommands(const string command);

	//************************************
	// Method:    lcd
	// FullName:  IHandleCommand::lcd
	// Access:    public
	// Returns:   void
	// Qualifier: execute LCD Command
	// Parameter: const string command
	//************************************
	void lcd(const string command);

	//************************************
	// Method:    pwd
	// FullName:  IHandleCommand::pwd
	// Access:    public
	// Returns:   void
	// Qualifier: execute PWD Command
	//************************************
	void pwd();

	//************************************
	// Method:    nonPortRelatedCommands
	// FullName:  IHandleCommand::nonPortRelatedCommands
	// Access:    public
	// Returns:   void
	// Qualifier: execute commands which require 1 argument (DELETE,CD,MKDIR,RMDIR)
	// Parameter: const string command
	// Parameter: const string noti
	// Parameter: const int commandLength
	// Parameter: const char * format
	//************************************
	void nonPortRelatedCommands(const string command, const string noti, const int commandLength, const char * format);
};