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

	// �򿪳ɹ���˵���Ѿ���ʵ��������
	if (NULL != semhd)
	{
		//printf("%s is already running.\n", sem_name);
		return false;
	}
	// ��ʧ�ܣ���˵���������������
	// �����ź���
	if (NULL == CreateSemaphoreA(NULL, 1, 1, sem_name))
	{
		printf("Create [%s] failed.\n", sem_name);
		return false;
	}

	return true;
}


bool StartMyService()
{

	HANDLE  semhd = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, FALSE, MASTER_APP_NAME);

	// �򿪳ɹ���˵���Ѿ���ʵ��������
	if (NULL != semhd)
	{
		CloseHandle(semhd);
		printf("%s is already running.\n", MASTER_APP_NAME);
		return true;
	}


	STARTUPINFOA   StartupInfo;		//���������������Ϣ�ṹ����    
	PROCESS_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	// Start the child process

	char cmd[MAX_PATH];
	char curDir[MAX_PATH];

	memset(cmd, 0, MAX_PATH);
	memset(curDir, 0, MAX_PATH);

	GetCurrentDirectoryA(MAX_PATH, curDir);


	snprintf(cmd, MAX_PATH, "%s\\%s --autostart", curDir, MASTER_APP_NAME);
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
	GetModuleFileNameA(NULL, strModule, MAX_PATH); //�õ���ǰģ��·��
	strcat(strModule, "\\..\\");     //����Ϊ��ǰ����·��Ϊ��ʱ����һ��
	SetCurrentDirectoryA(strModule);
}


int main()
{

	SetWorkPath();
	StartMyService();

	return 0;
}
