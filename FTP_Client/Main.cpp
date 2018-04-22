#include "stdafx.h"
#include "FTP_Client.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object
CWinApp theApp;

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

			ResponseErrorException ex;

			try
			{
				// Tao socket dau tien
				CSocket ClientSocket;
				ClientSocket.Create();

				// Ket noi den Server
				if (ClientSocket.Connect(_T(server), 21) != 0)
				{
					cout << "Ket noi toi Server thanh cong !!!" << endl << endl;
				}
				else
					return FALSE;

				char buf[BUFSIZ + 1];
				int tmpres, size, status;
				/*
				Connection Establishment
				120
				220
				220
				421
				Login
				USER
				230
				530
				500, 501, 421
				331, 332
				PASS
				230
				202
				530
				500, 501, 503, 421
				332
				*/
				char * str;
				int codeftp;
				printf("Connection established, waiting for welcome message...\n");

				//How to know the end of welcome message: http://stackoverflow.com/questions/13082538/how-to-know-the-end-of-ftp-welcome-message
				memset(buf, 0, sizeof buf);
				while ((tmpres = ClientSocket.Receive(buf, BUFSIZ, 0)) > 0) {
					sscanf(buf, "%d", &codeftp);
					printf("%s", buf);
					if (codeftp != 220) //120, 240, 421: something wrong
					{
						ex.setErrorCode(codeftp);
						throw ex;
					}

					str = strstr(buf, "220");//Why ???
					if (str != NULL) {
						break;
					}
					memset(buf, 0, tmpres);
				}

				//Send Username
				char info[50];
				printf("Name (%s): ", server);
				memset(buf, 0, sizeof buf);
				scanf("%s", info);

				sprintf(buf, "USER %s\r\n", info);
				tmpres = ClientSocket.Send(buf, strlen(buf), 0);

				memset(buf, 0, sizeof buf);
				tmpres = ClientSocket.Receive(buf, BUFSIZ, 0);

				sscanf(buf, "%d", &codeftp);
				if (codeftp != 331)
				{
					ex.setErrorCode(codeftp);
					throw ex;
				}
				printf("%s", buf);

				//Send Password
				memset(info, 0, sizeof info);
				printf("Password: ");
				memset(buf, 0, sizeof buf);
				scanf("%s", info);

				sprintf(buf, "PASS %s\r\n", info);
				tmpres = ClientSocket.Send(buf, strlen(buf), 0);

				memset(buf, 0, sizeof buf);
				tmpres = ClientSocket.Receive(buf, BUFSIZ, 0);

				sscanf(buf, "%d", &codeftp);
				if (codeftp != 230)
				{
					ex.setErrorCode(codeftp);
					throw ex;
				}
				printf("%s", buf);

				ClientSocket.Close();
			}
			catch (ResponseErrorException &e)
			{
				cout << e.getErrorStringResponse() << endl;
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