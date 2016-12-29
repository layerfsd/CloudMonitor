// MonitorService.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <tlhelp32.h>
#include <signal.h>
#include <time.h>

typedef void(*SignalHandlerPointer)(int);
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
		// 忽略大小写
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


#define DEPEND_APP_NAME		"FilterCenter.exe"

bool TryStartUp()
{
	DWORD dwPid;
	if (FindProcessPid(DEPEND_APP_NAME, dwPid))
	{
		printf("[%s] [%d]\n", DEPEND_APP_NAME, dwPid);
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

void EnableLog()
{
	// 生成日志文件名，取当天日期 YYYY-MM-DD.txt
	char LogName[MAX_PATH];
	FILE *stream;
	time_t timep;
	struct tm *p;


	time(&timep);
	p = localtime(&timep);
	memset(LogName, 0, sizeof(LogName));
	snprintf(LogName, MAX_PATH, "LOG\\%d-%d-%d[IO].txt", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday);


	if ((stream = freopen(LogName, "a+", stdout)) == NULL)
	{
		exit(-1);
	}

	memset(LogName, 0, sizeof(LogName));
	snprintf(LogName, MAX_PATH, "LOG\\IO-%d-%d-%d %02d:%02d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min);
	printf("\n\n\n\n\n[Start Time] %s\n", LogName);

}



int main(int argc, char *argv[])
{

	if (2 == argc && !strncmp(argv[1], "--backend", 7))
	{
		EnableLog();
	}

	printf("try start up\n");


	SetWorkPath();

	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGINT, SignalHandler);
	signal(SIGABRT, SignalHandler);
	SetHookOn();

	//SendMsg2Backend();
	//while (1)
	//{
	//	Sleep(30 * 1000);
	//	printf("helo\n");
	//}
	printf("main-end\n");
	return 0;
}

