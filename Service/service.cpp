 // filename: service.cpp
#include "service.h"
#include <string.h>

bool GetInstalledPath(char* InstallPath, size_t BufSize)
{
	const char *path = "Service.txt";

	strncpy(InstallPath, path, BufSize);
	return true;
}


int WriteToLog(char* str)
{
	static char LogFile[MAX_PATH];
	GetInstalledPath(LogFile, sizeof(LogFile));

	FILE* log;
	fopen_s(&log, LogFile, "a+");

	if (log == NULL)
		return -1;
	
	fprintf(log, "%s\n", str);
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


	// The worker loop of a service
	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		char *buffer[] {
			"[Filed] Start CloudMonitor",
			"[Success] Start CloudMonitor"
		};

		bool bRet = StartMyService();
		
		int result = WriteToLog(buffer[bRet]);
		if (result)
		{
			ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			ServiceStatus.dwWin32ExitCode = -1;
			SetServiceStatus(hStatus, &ServiceStatus);
			return;
		}
		Sleep(1000 * SLEEP_TIME);
	}
	return;
}

// Service initialization
int InitService()
{
	SetWorkPath();
	
	int result;
	result = WriteToLog("Monitoring started.");
	return(result);
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

bool StartMyService()
{

	HANDLE  semhd = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, FALSE, MASTER_APP_NAME);

	// 打开成功，说明已经有实例在运行
	if (NULL != semhd)
	{
		CloseHandle(semhd);
		return true;
	}


	STARTUPINFOA   StartupInfo;		//创建进程所需的信息结构变量    
	PROCESS_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);


	char cmd[MAX_PATH];
	char curDir[MAX_PATH];

	memset(cmd, 0, MAX_PATH);
	memset(curDir, 0, MAX_PATH);

	GetCurrentDirectoryA(MAX_PATH, curDir);

	printf("starting %s\n", MASTER_APP_NAME);

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
	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "\\..\\");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectory(strModule);
	GetCurrentDirectory(sizeof(strModule), strModule);
}
