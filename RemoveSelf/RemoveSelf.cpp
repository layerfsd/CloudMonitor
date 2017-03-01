// Uninstlal.cpp 
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <shellapi.h>
#include <shlobj.h>

#define ArraySize(ptr)		(sizeof(ptr) / sizeof(ptr[0]))

using namespace std;

static bool MyCreateProcess(LPCSTR appName, DWORD isShow=CREATE_NO_WINDOW)
{
	STARTUPINFOA   StartupInfo;		//创建进程所需的信息结构变量    
	PROCESS_INFORMATION pi;
	char output[MAXBYTE];

	if (NULL == appName)
	{
		return false;
	}

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	snprintf(output, sizeof(output), "%s", appName);

	if (CreateProcessA(NULL,
		output,
		NULL,
		NULL,
		FALSE,
		//0,
		isShow,
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


int main()
{
	const char *cmds[]{
		"sc stop CloudMonitorService",
		"taskkill /f /im CloudMonitor.exe",
		"taskkill /f /im MonitorService.exe",
		"taskkill /f /im MonitorService-64.exe",
		"sc delete CloudMonitorService",
	};

	for (int i = 0; i < ArraySize(cmds); i++)
	{
		MyCreateProcess(cmds[i]);
	}

	MyCreateProcess("msiexec.exe /x {BD79940C-6C20-46C2-B7CF-40D6D74C9A7D} /q", CREATE_NEW_CONSOLE);
	
	return 0;
}
