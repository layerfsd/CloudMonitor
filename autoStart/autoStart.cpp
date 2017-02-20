// autoStart.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>

#define INSTALL_WINDOW_TITLE "��ȫ��ܿͻ���"


void SetWorkPath()
{
	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //�õ���ǰģ��·��
	strcat(strModule, "\\..\\");     //����Ϊ��ǰ����·��Ϊ��ʱ����һ��
	SetCurrentDirectory(strModule);
}



bool MyCreateProcess(LPCSTR appName, LPSTR appArgs)
{
	STARTUPINFOA   StartupInfo;		//���������������Ϣ�ṹ����    
	PROCESS_INFORMATION pi;
	char output[MAXBYTE];

	if (NULL == appName)
	{
		return false;
	}

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	snprintf(output, sizeof(output), "%s %s", appName, appArgs);

	if (CreateProcessA(NULL,
		output,
		NULL,
		NULL,
		FALSE,
		//0,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&StartupInfo,
		&pi))
	{

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		snprintf(output, MAXBYTE, "[ERROR] CreateProcess: %s", appName);
		return false;
	}

	return true;
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
		// ���Դ�Сд
		if (!_stricmp(ProcessName, pe32.szExeFile))
		{
			dwPid = pe32.th32ProcessID;
			bRet = TRUE;
			break;
		}

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return bRet;
}

// �رհ�װҳ��
int killInstallWindow()
{
	HWND hnw = FindWindow(NULL, INSTALL_WINDOW_TITLE);
	if (NULL == hnw) {
		return -1;
	}
	DWORD dnp = 0;
	GetWindowThreadProcessId(hnw, &dnp);
	if (0 == dnp) {
		return -1;
	}
	HANDLE hnh = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dnp);
	if (NULL == hnh) {
		return -1;
	}
	TerminateProcess(hnh, 0);
	CloseHandle(hnw);

	return 0;
}


int __stdcall WinMain(HINSTANCE hInstance,      // handle to current instance
	HINSTANCE hPrevInstance,  // handle to previous instance
	LPSTR lpCmdLine,          // command line
	int nCmdShow              // show state
	)
{
	killInstallWindow();
	SetWorkPath();
	MyCreateProcess("Monitor.exe", NULL);
    return 0;
}

