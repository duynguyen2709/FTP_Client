#include "stdafx.h"
#include "FTP_Client.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object
CWinApp theApp;

#define ftp_str "ftp>"

inline string FormatCommand(string command)
{
	//DELETE SPACES AT END
	int pos = command.size() - 1;
	while (command[pos] == ' ') {
		command.erase(pos);
		pos--;
	}

	//DELETE SPACES AT START
	pos = 0;
	while (command[pos] == ' ') {
		command.erase(pos);
		pos++;
	}

	//FORMAT COMMAND TO LOWERCASE
	int spacePos = command.find_first_of(' ');

	if (spacePos != string::npos) {
		for (int i = 0; i < spacePos; i++)
			command[i] = tolower(command[i]);
	}
	else transform(command.begin(), command.end(), command.begin(), ::tolower);

	return command;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
			// Khoi tao thu vien Socket
			if (AfxSocketInit() == FALSE)
			{
				cout << "Khong the khoi tao Socket Library";
				return FALSE;
			}
			srand(time(NULL));

			//
			//START OF MAIN CODE
			//
			ResponseErrorException::InitErrorCodeList();
			FTP_Client::initCommandList();

			FTP_Client client;
			ResponseErrorException ex;
			string command;

			cout << "Enter FTP command. \"?\" or \"help\" for command help" << endl;
			while (1)
			{
			enter: cout << ftp_str;
				try {
					getline(cin, command);

					command = FormatCommand(command);

					if (command == "quit" || command == "exit")
					{
						exit(0);
					}

					//if not connected to server
					//then check if command = "open" or "ftp" to establish connection,
					//otherwise every other command can not be executed.
					else if (client.isConnected() == false)
					{
						if (command.find("open") != string::npos || command.find("ftp") != string::npos)
						{
							bool status = client.login(command);
							if (!status)
							{
								ex.setErrorCode(530);
								throw ex;
							}
						}

						else {

							//
							//check if command is legit
							//
							if (FTP_Client::checkCommand(command))
							{
								ex.setErrorCode(0);
								throw ex;
							}
							else {
								ex.setErrorCode(1);
								throw ex;
							}
						}
					}
					else {
						if (FTP_Client::checkCommand(command) == false)
						{
							ex.setErrorCode(1);
							throw ex;
						}

						client.executeCommand(command);
					}
				}
				catch (ResponseErrorException &e) {
					cout << e.getErrorStringResponse() << endl;
					goto enter;
				}
			}
		}
	}

	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}