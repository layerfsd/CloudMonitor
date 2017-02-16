#include "stdafx.h"
#include <windows.h>

#define ArraySize(ptr)		(sizeof(ptr) / sizeof(ptr[0]))

bool SysRun(const char* cmd)
{
	FILE* execfd = NULL;

	printf("cmd: [%s]\n", cmd);

	execfd = _popen(cmd, "r");

	if (NULL == execfd)
	{
		return false;
	}

	return (0 == _pclose(execfd));
}


bool MyCreateProcess(LPCSTR appName, LPSTR appArgs=NULL)
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


int __stdcall WinMain(HINSTANCE hInstance,      // handle to current instance
	HINSTANCE hPrevInstance,  // handle to previous instance
	LPSTR lpCmdLine,          // command line
	int nCmdShow              // show state
	)
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
    return 0;
}

