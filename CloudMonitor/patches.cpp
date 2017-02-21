
// Use signal to attach a signal handler to the abort routine
#include "FileMon.h"
#include "network.h"
#include "patches.h"
#include "AutoStart.h"
#include "process.h"

#include <signal.h>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <io.h>
#include <tlhelp32.h>

#define THIS_APP_NAME			"CloudMonitor.exe"
#define DEPEND_APP_NAME			"MonitorService.exe"
#define DEPEND_APP_NAME_64		"MonitorService-64.exe"

#define BACKEND_FLAG			"--backend"


using namespace std;

extern BOOL g_RUNNING;


bool InformUser(int info);


// 检查守护服务是否开启
void CheckDaemonService()
{
	if (!IsServiceRunning())
	{
		// 判断是否自更新程序正在运行
		if ((_access(UPDATE_CHECKED_FLAG, 0) != 0))
		{
			WriteToLog("[CloudMonitor] " SERVICE_NAME " is not runing");
			WriteToLog("[CloudMonitor] I will start the Service.exe");
			DoStartSvc(SERVICE_NAME);
		}
	}

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



bool MyCreateProcess(LPCSTR appName, LPSTR appArgs = NULL)
{
	STARTUPINFOA   StartupInfo;		//创建进程所需的信息结构变量    
	PROCESS_INFORMATION pi;
	char output[MAXBYTE];
	char cmd[MAXBYTE];

	if (NULL == appName)
	{
		return false;
	}

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	snprintf(cmd, sizeof(output), "%s %s", appName, appArgs);

	WriteToLog(output);

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
		snprintf(output, MAXBYTE, "[ERROR] CreateProcess: %s", appName);
		WriteToLog(output);
		return false;
	}

	return true;

}


typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL IsWow64()
{
	BOOL bIsWow64 = FALSE;

	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.

	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			//handle error
		}
	}
	return bIsWow64;
}


bool StartHookService()
{

	DWORD	dwPid;

	// 找不到目标进程时，才启动
	if (!FindProcessPid(DEPEND_APP_NAME, dwPid))
	{
		MyCreateProcess(DEPEND_APP_NAME, BACKEND_FLAG);
		WriteToLog("[CloudMonitor starting] " DEPEND_APP_NAME);
	}
	
	// 检测系统是否支持64位程序运行
	if (IsWow64() && !FindProcessPid(DEPEND_APP_NAME_64, dwPid))
	{
		// 本hook模块在win7 64 位上运行崩溃
		// 在win8、win10 上正常
		if ( !IsWin7())
		{
			MyCreateProcess(DEPEND_APP_NAME_64, BACKEND_FLAG);
		}
		WriteToLog("[CloudMonitor starting] " DEPEND_APP_NAME_64);
	}
	return true;
}

void SetWorkPath()
{
	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "\\..\\");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectory(strModule);
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


	GetMyName(sem_name, MAX_PATH);
	
	if (!TryStartUp(sem_name))
	{
		InformUser(ALREADY_LOGIN);
		exit(3);
	}

	RegSigint(); //注册 CTRL+C 信号处理函,正常终止会话.

	//StartHookService();

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
			cout << dirs[i] << " not exists!!!" << endl;
			cout << "mkdir: " << dirs[i] << endl;
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



int WriteToLog(char* str)
{
	static char LogFile[] = "Service.txt";
	static char timeBuf[MAX_PATH];


	time_t timep;
	struct tm *p;


	time(&timep);
	p = localtime(&timep);
	memset(timeBuf, 0, sizeof(timeBuf));
	snprintf(timeBuf, MAX_PATH, "[%d-%02d-%02d %02d:%02d:%02d] ", \
		1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);


	FILE* log;
	fopen_s(&log, LogFile, "a+");

	if (log == NULL)
		return -1;

	fprintf(log, "%s%s\n", timeBuf, str);
	fclose(log);
	return 0;
}

void CleanTmpFiles(SFile& file)
{
	remove(file.savedPath.c_str());		// tmp\doc
	remove(file.encPath.c_str());		// tmp\aes
	remove(file.txtPath.c_str());		// tmp\txt
}


//Version Number    Description
//6.1               Windows 7     / Windows 2008 R2
//6.0               Windows Vista / Windows 2008
//5.2               Windows 2003 
//5.1               Windows XP
//5.0               Windows 2000
#pragma warning(disable:4996)
bool IsWin7()
{
	LPOSVERSIONINFO lpVersionInfo;
	DWORD version = GetVersionExA(lpVersionInfo);

	return ((lpVersionInfo->dwMajorVersion == 6) && (lpVersionInfo->dwMinorVersion == 1));
}



//
// Purpose: 
//   Starts the service if possible.
//
// Return value:
//   None
//
VOID __stdcall DoStartSvc(const char* szSvcName)
{
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;

	// Get a handle to the SCM database. 

	auto schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		//printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service.

	auto schService = OpenServiceA(
		schSCManager,         // SCM database 
		szSvcName,            // name of service 
		SERVICE_ALL_ACCESS);  // full access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Check the status in case the service is not stopped. 

	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // information level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // size needed if buffer is too small
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// Check if the service is already running. It would be possible 
	// to stop the service here, but for simplicity this example just returns. 

	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		printf("Cannot start the service because it is already running\n");
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	// Wait for the service to stop before attempting to start it.

	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status until the service is no longer stop pending. 

		if (!QueryServiceStatusEx(
			schService,                     // handle to service 
			SC_STATUS_PROCESS_INFO,         // information level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // size needed if buffer is too small
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				printf("Timeout waiting for service to stop\n");
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return;
			}
		}
	}

	// Attempt to start the service.

	if (!StartService(
		schService,  // handle to service 
		0,           // number of arguments 
		NULL))      // no arguments 
	{
		printf("StartService failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}
	else printf("Service start pending...\n");

	// Check the status until the service is no longer start pending. 

	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // if buffer too small
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status again. 

		if (!QueryServiceStatusEx(
			schService,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // if buffer too small
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			break;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				// No progress made within the wait hint.
				break;
			}
		}
	}

	// Determine whether the service is running.

	if (ssStatus.dwCurrentState == SERVICE_RUNNING)
	{
		printf("Service started successfully.\n");
	}
	else
	{
		printf("Service not started. \n");
		printf("  Current State: %d\n", ssStatus.dwCurrentState);
		printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
		printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
		printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}


int GetServiceStatus(const char* name)
{
	SC_HANDLE theService, scm;
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwBytesNeeded;


	scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
	if (!scm) {
		return 0;
	}

	theService = OpenServiceA(scm, name, SERVICE_QUERY_STATUS);
	if (!theService) {
		CloseServiceHandle(scm);
		return 0;
	}

	auto result = QueryServiceStatusEx(theService, SC_STATUS_PROCESS_INFO,
		reinterpret_cast<LPBYTE>(&ssStatus), sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded);

	CloseServiceHandle(theService);
	CloseServiceHandle(scm);

	if (result == 0) {
		return 0;
	}

	return ssStatus.dwCurrentState;
}


bool IsServiceRunning()
{
	return SERVICE_RUNNING == GetServiceStatus(SERVICE_NAME);
}