
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
#define DEPEND_APP_NAME		"MonitroService.exe"


using namespace std;

extern BOOL g_RUNNING;


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



bool TryStartUp(const char* sem_name)
{
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

BOOL DeleteDirectory(const char * DirName)
{
	//	CFileFind tempFind;		//声明一个CFileFind类变量，以用来搜索
	char szCurPath[MAX_PATH];		//用于定义搜索格式
	_snprintf(szCurPath, MAX_PATH, "%s//*.*", DirName);	//匹配格式为*.*,即该目录下的所有文件
	WIN32_FIND_DATAA FindFileData;
	ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));
	HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData);
	BOOL IsFinded = TRUE;
	while (IsFinded)
	{
		IsFinded = FindNextFileA(hFile, &FindFileData);	//递归搜索其他的文件
		if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, "..")) //如果不是"." ".."目录
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

	// 打开成功，说明已经有实例在运行
	if (NULL != semhd)
	{
		CloseHandle(semhd);
		printf("%s is already running.\n", DEPEND_APP_NAME);
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
	snprintf(LogName, MAX_PATH, "LOG\\%d-%d-%d.txt", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday);


	// 隐藏控制台窗口
	if (hide)
	{
		ShowWindow(GetConsoleWindow(), SW_HIDE);
		// 如果为开机自启动，先休眠30秒，等待系统连接网络
		Sleep(30000);
		if ((stream = freopen(LogName, "a+", stdout)) == NULL)
		{
			exit(-1);
		}
	}


	GetMyName(sem_name, MAX_PATH);
	
	if (!TryStartUp(sem_name))
	{
		InformUser(ALREADY_LOGIN);

		exit(3);
	}
	RegSigint(); //注册 CTRL+C 信号处理函,正常终止会话.


	SetWorkPath();
	StartHookService();

	// 检查本程序是否已经加入到开机自启动项目
	RegisterProgram();

	// 创建临时目录
	if (-1 == _access(TMP_DIR, 0))
	{
		cout << TMP_DIR << " not exists!!!" << endl;
		cout << "mkdir: " << TMP_DIR << endl;
		_mkdir(TMP_DIR);
	}
	else
	{
		DeleteDirectory(TMP_DIR);	// 程序每次启动时,清空所有临时文件
	}
	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// 创建一个本地 TCP 端口,接收敏感事件

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