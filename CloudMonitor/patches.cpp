
// Use signal to attach a signal handler to the abort routine
#include "headers\FileMon.h"
#include "patches.h"
#include <signal.h>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <io.h>
#include <tlhelp32.h>


#define THIS_APP_NAME		"CloudMonitor.exe"
#define DEPEND_APP_NAME		"MonitroService.exe"


using namespace std;

extern BOOL g_RUNNING;



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


void SignalHandler(int signal)
{
	//system("cls");
	printf("\nCloudMonitor Exciting...\n");
	g_RUNNING = FALSE;
	//exit(signal);
	return;
}


void regSINGINT()
{
	typedef void(*SignalHandlerPointer)(int);

	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGINT, SignalHandler);

	return;
}

BOOL IsDirectory(const char *pDir)
{
	char szCurPath[500];
	ZeroMemory(szCurPath, 500);
	sprintf_s(szCurPath, 500, "%s//*", pDir);
	WIN32_FIND_DATAA FindFileData;
	ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));

	HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData); /**< find first file by given path. */

	if (hFile == INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		return FALSE; /** ��������ҵ���һ���ļ�����ôû��Ŀ¼ */
	}
	else
	{
		FindClose(hFile);
		return TRUE;
	}

}

BOOL DeleteDirectory(const char * DirName)
{
	//	CFileFind tempFind;		//����һ��CFileFind�����������������
	char szCurPath[MAX_PATH];		//���ڶ���������ʽ
	_snprintf(szCurPath, MAX_PATH, "%s//*.*", DirName);	//ƥ���ʽΪ*.*,����Ŀ¼�µ������ļ�
	WIN32_FIND_DATAA FindFileData;
	ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));
	HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData);
	BOOL IsFinded = TRUE;
	while (IsFinded)
	{
		IsFinded = FindNextFileA(hFile, &FindFileData);	//�ݹ������������ļ�
		if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, "..")) //�������"." ".."Ŀ¼
		{
			string strFileName = "";
			strFileName = DirName;
			strFileName += FindFileData.cFileName;
			DeleteFileA(strFileName.c_str());
		}
	}
	FindClose(hFile);

	return TRUE;
}


bool StartHookService()
{

	HANDLE  semhd = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, FALSE, DEPEND_APP_NAME);

	// �򿪳ɹ���˵���Ѿ���ʵ��������
	if (NULL != semhd)
	{
		printf("%s is already running.\n", DEPEND_APP_NAME);
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

	strcat(cmd, "\\MonitroService.exe");

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

void InitDir()
{
	char	sem_name[MAX_PATH];

	memset(sem_name, 0, MAX_PATH);
	GetMyName(sem_name, MAX_PATH);
	
	if (!TryStartUp(sem_name))
	{
		exit(3);
	}
	regSINGINT(); //ע�� CTRL+C �źŴ���,������ֹ�Ự.


	SetWorkPath();
	StartHookService();

	// ������ʱĿ¼
	if (-1 == _access(TMP_DIR, 0))
	{
		cout << TMP_DIR << " not exists!!!" << endl;
		cout << "mkdir: " << TMP_DIR << endl;
		_mkdir(TMP_DIR);
	}
	else
	{
		DeleteDirectory(TMP_DIR);	// ����ÿ������ʱ,���������ʱ�ļ�
	}
	return;
}