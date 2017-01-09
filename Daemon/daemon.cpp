#include <signal.h>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <io.h>
#include <tlhelp32.h>
#include <stdio.h>

#define MASTER_APP_NAME		"CloudMonitor.exe"

static bool g_RUNNING = true;

using namespace std;

bool GetMyName(char* szBuf, size_t bufSize)
{

	CHAR    szPath[MAX_PATH] = { 0 };

	if (!GetModuleFileNameA(NULL, szPath, MAX_PATH))
	{
		printf("GetModuleFileName failed (%d)\n", GetLastError());
		return false;
	}

	char* pos = NULL;

	pos = strrchr(szPath, '\\');

	if (NULL == pos)
	{
		return false;
	}

	strncpy(szBuf, pos + 1, bufSize);
	return true;
}



bool TryStartUp(const char* sem_name)
{
	//printf("sem_name: %s\n", sem_name);

	HANDLE  semhd = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, FALSE, sem_name);

	// 打开成功，说明已经有实例在运行
	if (NULL != semhd)
	{
		//printf("%s is already running.\n", sem_name);
		return false;
	}
	// 打开失败，则说明本程序初次启动
	// 创建信号量
	if (NULL == CreateSemaphoreA(NULL, 1, 1, sem_name))
	{
		printf("Create [%s] failed.\n", sem_name);
		return false;
	}

	return true;
}


bool StartMyService(const char* args)
{

	HANDLE  semhd = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, FALSE, MASTER_APP_NAME);

	// 打开成功，说明已经有实例在运行
	if (NULL != semhd)
	{
		CloseHandle(semhd);
		printf("%s is already running.\n", MASTER_APP_NAME);
		return true;
	}


	STARTUPINFOA   StartupInfo;		//创建进程所需的信息结构变量    
	PROCESS_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	// Start the child process

	char cmd[MAX_PATH];

	memset(cmd, 0, MAX_PATH);

	snprintf(cmd, MAX_PATH, "%s %s", MASTER_APP_NAME, args);

	printf("[CMD] %s\n", cmd);
	if (CreateProcessA(NULL,
		cmd,
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
		printf("[OK] CreateProcess: %s\n", cmd);
	}
	else
	{
		printf("[ERROR] CreateProcess: %s\n", cmd);
		return false;
	}
	return true;
}

void SetWorkPath()
{
	char strModule[MAX_PATH];
	GetModuleFileNameA(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "\\..\\");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectoryA(strModule);
	GetModuleFileNameA(NULL, strModule, MAX_PATH); //得到当前模块路径
}

int __stdcall WinMain(HINSTANCE hInstance,      // handle to current instance
	HINSTANCE hPrevInstance,  // handle to previous instance
	LPSTR lpCmdLine,          // command line
	int nCmdShow              // show state
	)
{

	SetWorkPath();
	StartMyService(lpCmdLine);

	return 0;
}
