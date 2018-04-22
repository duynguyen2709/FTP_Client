#include "stdafx.h"
#include "FTP_Client.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object
CWinApp theApp;

#define ftp_str "ftp>"

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
				cout << "Khong the khoi tao Socket Libraray";
				return FALSE;
			}

			//
			//START OF MAIN CODE
			//
			ResponseErrorException::InitErrorCodeList();
			cout << "Enter FTP command. ? for help" << endl;
		enter: cout << ftp_str;
			string command;
			getline(cin, command);
			std::transform(command.begin(), command.end(), command.begin(), ::tolower);

			if (command.find("open") != string::npos)
				FTP_Client *client = new FTP_Client();
			else {
				cout << "Not connected" << endl;
				goto enter;
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