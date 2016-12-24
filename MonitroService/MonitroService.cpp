// MonitroService.cpp : �������̨Ӧ�ó������ڵ㡣
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

	// �򿪳ɹ���˵���Ѿ���ʵ��������
	if (NULL != semhd)
	{
		printf("%s is already running.\n", sem_name);
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

void SetWorkPath()
{
	char strModule[MAX_PATH];
	GetModuleFileNameA(NULL, strModule, MAX_PATH); //�õ���ǰģ��·��
	strcat(strModule, "\\..\\");     //����Ϊ��ǰ����·��Ϊ��ʱ����һ��
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

