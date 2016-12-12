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


void SignalHandler(int signal)
{
	//system("cls");
	printf("\nCloudMonitor Exciting...\n");
	g_RUNNING = FALSE;
	//exit(signal);
	return;
}


void RegSigint()
{
	typedef void(*SignalHandlerPointer)(int);

	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGINT, SignalHandler);

	return;
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
	memset(cmd, 0, MAX_PATH);
	GetCurrentDirectoryA(MAX_PATH, cmd);

	strcat(cmd, "\\");
	strcat(cmd, MASTER_APP_NAME);
    printf("CreateProcess %s\n", cmd);	
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
	GetModuleFileName(NULL, strModule, MAX_PATH); //�õ���ǰģ��·��
	strcat(strModule, "\\..\\");     //����Ϊ��ǰ����·��Ϊ��ʱ����һ��
	SetCurrentDirectory(strModule);
	GetCurrentDirectory(sizeof(strModule), strModule);
}


void TryStartMySelf()
{
	char	sem_name[MAX_PATH];

	memset(sem_name, 0, MAX_PATH);
	GetMyName(sem_name, MAX_PATH);

	if (!TryStartUp(sem_name))
	{
		//printf("[Daemon: %s] is running.\n", sem_name);
		exit(3);
	}
	RegSigint(); //ע�� CTRL+C �źŴ���,������ֹ�Ự.


	SetWorkPath();
	return;
}


int main()
{
	TryStartMySelf();


	while (g_RUNNING)
	{
		Sleep(1000);
		if (TryStartUp(MASTER_APP_NAME))
		{
			StartMyService();
		}
	}
	return 0;
}
