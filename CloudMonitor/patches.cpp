
// Use signal to attach a signal handler to the abort routine
#include "FileMon.h"
#include "network.h"
#include "patches.h"
#include "AutoStart.h"

#include <signal.h>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <io.h>
#include <tlhelp32.h>


#define THIS_APP_NAME		"CloudMonitor.exe"
#define DEPEND_APP_NAME		"FilterCenter.exe"


using namespace std;

extern BOOL g_RUNNING;

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

bool InformUser(int info);

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
		return FALSE; /** 如果不能找到第一个文件，那么没有目录 */
	}
	else
	{
		FindClose(hFile);
		return TRUE;
	}

}


bool StartHookService()
{
	DWORD dwPid;

	if (FindProcessPid(DEPEND_APP_NAME, dwPid))
	{
		printf("[%s] started before.", DEPEND_APP_NAME);
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
	GetCurrentDirectoryA(MAX_PATH, cmd);

	strcat(cmd, "\\FilterCenter.exe");
	strcat(cmd, " --backend");

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
	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "\\..\\");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectory(strModule);
	GetCurrentDirectory(sizeof(strModule), strModule);
}

void InitDir(bool hide)
{
	char	sem_name[MAX_PATH];
	// 生成日志文件名，取当天日期 YYYY-MM-DD.txt
	char LogName[MAX_PATH];
	FILE *stream;
	time_t timep;
	struct tm *p;

	memset(sem_name, 0, MAX_PATH);

	time(&timep);
	p = localtime(&timep);
	memset(LogName, 0, sizeof(LogName));
	snprintf(LogName, MAX_PATH, "LOG\\%d-%d-%d[Client].txt", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday);


	// 隐藏控制台窗口
	if (hide)
	{
		ShowWindow(GetConsoleWindow(), SW_HIDE);
		if ((stream = freopen(LogName, "a+", stdout)) == NULL)
		{
			exit(-1);
		}
	}

	memset(LogName, 0, sizeof(LogName));
	snprintf(LogName, MAX_PATH, "LOG\\%d-%d-%d %02d:%02d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min);
	printf("\n\n\n\n\n[Start Time] %s\n", LogName);



	RegSigint(); //注册 CTRL+C 信号处理函,正常终止会话.


	SetWorkPath();
	StartHookService();

	// 检查本程序是否已经加入到开机自启动项目
	// RegisterProgram();

	// 创建临时目录
	const char *dirs[] = {
		"DATA", "LOG", "TMP",
	};

	for (int i = 0; i < ArraySize(dirs); i++)
	{
		if (-1 == _access(dirs[i], 0))
		{
			cout << TMP_DIR << " not exists!!!" << endl;
			cout << "mkdir: " << TMP_DIR << endl;
			_mkdir(dirs[i]);
		}
	}
	system("del /F /S /Q TMP"); 	// 程序每次启动时,清空临时目录

	return;
}


bool InformUser(int info)
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(159);
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("[INFORM] connect error !");
		closesocket(sclient);
		return 0;
	}

	char Buf[4] = { 0, 0, 0, 0 };

	sprintf(Buf, "%d", info);
	
	if (!send(sclient, Buf, strlen(Buf), 0))
	{
		cout << "写入数据失败 ..." << endl << endl;
		return false;
	}
	cout << "写入数据成功：    " << info << endl << endl;
	closesocket(sclient);
	WSACleanup();

	return true;
}