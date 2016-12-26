// Update.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

using namespace std;

LPCSTR TargetProcessName[]{ "CloudMonitor.exe", "MonitorService.exe" };


BOOL SendSignal2Process(DWORD dwPid, DWORD dwSig);
BOOL FindProcessPid(LPCSTR ProcessName, DWORD& dwPid);

VOID SendControlC(DWORD pid);

void logLastError()
{
	LPTSTR errorText = NULL;

	FormatMessage(
		// use system message tables to retrieve error text
		FORMAT_MESSAGE_FROM_SYSTEM
		// allocate buffer on local heap for error text
		| FORMAT_MESSAGE_ALLOCATE_BUFFER
		// Important! will fail otherwise, since we're not 
		// (and CANNOT) pass insertion parameters
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&errorText,  // output 
		0, // minimum size for output buffer
		NULL);   // arguments - see note 

	if (NULL != errorText)
	{
		printf("failure: %s", errorText);
		LocalFree(errorText);
		errorText = NULL;
	}
}


static inline VOID StopMyService()
{
	DWORD	dwPid = 0;
	BOOL	bRet = FALSE;
	LPCSTR  pos{ nullptr };

	
	for (int i = 0; i < ArraySize(TargetProcessName); i++)
	{
		pos = TargetProcessName[i];
		bRet = FindProcessPid(pos, dwPid);

		// 仅当找到进程pid时，才尝试关闭
		while (0 != bRet)
		{
			printf("Sending SIGINT to [%s] [%d]\n", pos, dwPid);
			SendControlC(dwPid);
			Sleep(2 * 1000);

			cout << "After sent SIGINT" << endl;
			bRet = FindProcessPid(pos, dwPid);
		}
	}

	return;
}


int main()
{
	StopMyService();
    return 0;
}

VOID SendControlC(DWORD pid)
{
	printf("sending ctrl+c to pid %d", pid); 
	FreeConsole(); 
	if (AttachConsole(pid))
	{
		SetConsoleCtrlHandler(NULL, true);
		GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0); 
	}
	else {
		logLastError();
	}
}

BOOL FindProcessPid(LPCSTR ProcessName, DWORD& dwPid)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return(FALSE);
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}

	BOOL	bRet = FALSE;
	do
	{
		if (!strncmp(ProcessName, pe32.szExeFile, MAX_PATH))
		{
			dwPid = pe32.th32ProcessID;
			bRet = TRUE;
			break;
		}

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return bRet;
}
