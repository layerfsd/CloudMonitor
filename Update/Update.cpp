// Update.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

//  access URL get file list 
//		list format: file_path md5
//  if exists file md5 diff, update the specific file

//  find CloudMonitor.exe pid
//  send SIGINT to the pid and wait process exit
//  replace the file and restart CloudMonitor.exe
using namespace std;

LPCSTR TargetProcessName = "CloudMonitor.exe";


BOOL FindProcessPid(LPCSTR ProcessName, DWORD& dwPid);

int main()
{
	DWORD	dwPid = 0;
	BOOL	bRet = FALSE;

	bRet = FindProcessPid(TargetProcessName, dwPid);
	
	cout << dwPid << endl;

    return 0;
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
			break;
		}

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return bRet;
}
