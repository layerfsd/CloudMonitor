 // filename: service.cpp
#include "service.h"
#include <io.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>	//CreateToolhelp32Snapshot

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

char Gcmd[MAX_PATH];

bool StartHookService();

inline void killCloudMonitor()
{
	DWORD dwPid = 0;

	if (FindProcessPid(MASTER_APP_NAME, dwPid))
	{
		HANDLE hnh = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		if (NULL != dwPid) {
			TerminateProcess(hnh, 0);
			CloseHandle(hnh);
			WriteToLog(MASTER_APP_NAME " KILLED OK");
		}
	}
}

BOOL WINAPI ConsoleHandler(DWORD event)
{
	if (CTRL_SHUTDOWN_EVENT)
	{
		// stop service
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;

		WriteToLog("Closing " MASTER_APP_NAME);
		//kill CloudMonitor.exe
		killCloudMonitor();

		WriteToLog("POWER OFF");
	}
	return TRUE;
}


bool MyCreateProcess(LPCSTR appName, LPSTR appArgs)
{
	STARTUPINFOA   StartupInfo;		//创建进程所需的信息结构变量    
	PROCESS_INFORMATION pi;
	char output[MAXBYTE];

	if (NULL == appName)
	{
		return false;
	}

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	snprintf(output, sizeof(output), "%s %s", appName, appArgs);
	WriteToLog(output);

	if (CreateProcessA(NULL,
		output,
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


int WriteToLog(char* str)
{
	static char LogFile[] = LOG_FILE_PATH;
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


void main()
{
	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = SERVICE_NAME;
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;
	// Start the control dispatcher thread for our service
	StartServiceCtrlDispatcher(ServiceTable);
}


void ServiceMain(int argc, char** argv)
{
	int error;

	ServiceStatus.dwServiceType = SERVICE_WIN32;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	hStatus = RegisterServiceCtrlHandler(
		SERVICE_NAME,
		(LPHANDLER_FUNCTION)ControlHandler);
	if (hStatus == (SERVICE_STATUS_HANDLE)0)
	{
		// Registering Control Handler failed
		return;
	}
	// Initialize Service 
	error = InitService();
	if (error)
	{
		// Initialization failed
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwWin32ExitCode = -1;
		SetServiceStatus(hStatus, &ServiceStatus);
		return;
	}
	// We report the running status to SCM. 
	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hStatus, &ServiceStatus);


	DWORD dwPid = 0;
	// The worker loop of a service
	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		Sleep(SLEEP_TIME);

		dwPid = 0;
		// 如果找不到该进程则启动之
		if (!FindProcessPid(MASTER_APP_NAME, dwPid))
		{
			// 以admin身份启动 CloudMonitor 进程
			if (MyCreateProcess(MASTER_APP_NAME, MASTER_APP_ARGS))
			{
				WriteToLog("[SERVICE-START] " MASTER_APP_NAME " OK");
			}
			else
			{
				WriteToLog("[SERVICE-START] " MASTER_APP_NAME " FAILED");
			}
		}
		else {
			StartHookService();
		}

		// end while
	}

	return;
}

bool IsUpdateChecked()
{
	bool ret = false;

	if ((_access(UPDATE_CHECKED_FLAG, 0) == 0))
	{
		WriteToLog("[SERVICE] " UPDATE_CHECKED_FLAG " Existed.");
		WriteToLog("[SERVICE] delete " UPDATE_CHECKED_FLAG);
		// 一旦检测到，‘标志文件’，立刻删除
		remove(UPDATE_CHECKED_FLAG);
		ret = true;
	}

	return ret;
}



static int GetFileSize(char *FileName, size_t *FileSize)
{
	FILE *fp;

	if ((fp = fopen(FileName, "rb")) == NULL)
		return -1;

	//printf("%s open OK\n", FileName);
	fseek(fp, 0, SEEK_END);
	*FileSize = ftell(fp);

	fclose(fp);
	fp = NULL;

	return 0;
}


bool SysRun(const char* cmd)
{
	FILE* execfd = NULL;

	printf("cmd: [%s]\n", cmd);

	execfd = _popen(cmd, "r");

	if (NULL == execfd)
	{
		return false;
	}

	return (0 == _pclose(execfd));
}

// 根据日志文件大小，决定是否删除由本软件产生的日志文件
void DecideCleanLogFile()
{
	size_t fileSize = 0;
	if (0 != GetFileSize(LOG_FILE_PATH, &fileSize))
	{
		return;
	}
	if (fileSize >= LOGFILE_MAX_SIZE)
	{
		SysRun("del /f LOG_FILE_PATH");
	}
}


// Service initialization
// 返回非0，则退出本服务
int InitService()
{
	SetWorkPath();
	
	int result;
	DWORD dwPid;

	DecideCleanLogFile();
	
	//snprintf(Gcmd, sizeof(Gcmd), "%s %s", MASTER_APP_NAME, MASTER_APP_ARGS);

	result = WriteToLog("Monitoring started.");
	if (SetConsoleCtrlHandler(
		(PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE)
	{
		// unable to install handler... 
		// display message to the user
		WriteToLog("Unable to install ConsoleHandler !\n");
		return -1;
	}


	if (!IsUpdateChecked())
	{
		WriteToLog("not found " UPDATE_CHECKED_FLAG " prepare to start " UPDATE_APP_NAME);
		MyCreateProcess(UPDATE_APP_NAME, UPDATE_ARGS);
		WriteToLog("stop service for updating ");
		return 1;
	}

	if (!FindProcessPid(MASTER_APP_NAME, dwPid))
	{
		if (MyCreateProcess(MASTER_APP_NAME, MASTER_APP_ARGS))
		{
			WriteToLog("[SERVICE-START] " MASTER_APP_NAME " OK");
		}
		else
		{
			WriteToLog("[SERVICE-START] " MASTER_APP_NAME " FAILED");
		}
	}

	return 0;
}


void SetWorkPath()
{
	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "\\..\\");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectory(strModule);
	GetCurrentDirectory(sizeof(strModule), strModule);
}

// Control handler function
void ControlHandler(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		WriteToLog("Monitoring stopped.");
		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hStatus, &ServiceStatus);
		return;

	case SERVICE_CONTROL_SHUTDOWN:
		WriteToLog("Monitoring stopped.");
		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hStatus, &ServiceStatus);
		return;

	default:
		break;
	}

	// Report current status
	SetServiceStatus(hStatus, &ServiceStatus);

	return;
}
