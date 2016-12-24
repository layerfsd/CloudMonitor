// MonitroService.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <signal.h>


typedef void(*SignalHandlerPointer)(int);

void SignalHandler(int signal)
{
	printf("\nMonitorService Exciting...\n");
	SetHookOff();
	HINSTANCE  m_hInst = GetModuleHandleA("HooKeyboard.dll");

	CloseHandle(m_hInst);
	FreeLibrary(m_hInst);
}

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



bool TryStartUp()
{
	char	sem_name[MAX_PATH];

	memset(sem_name, 0, MAX_PATH);
	if (!GetMyName(sem_name, MAX_PATH))
	{
		return false;
	}
	printf("sem_name: %s\n", sem_name);

	HANDLE  semhd = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, FALSE, sem_name);

	// 打开成功，说明已经有实例在运行
	if (NULL != semhd)
	{
		printf("%s is already running.\n", sem_name);
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

void SetWorkPath()
{
	char strModule[MAX_PATH];
	GetModuleFileNameA(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "\\..\\");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectoryA(strModule);
	GetCurrentDirectoryA(sizeof(strModule), strModule);
}

int main()
{
	if (!TryStartUp())
	{
		exit(3);
	}

	SetWorkPath();

	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGINT, SignalHandler);
	signal(SIGABRT, SignalHandler);
	SetHookOn();	
	printf("main-end\n");
	return 0;
}

