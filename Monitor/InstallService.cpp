#include "stdafx.h"
#include <winsvc.h>

#define SERVICE_NAME  "CloudMonitorService"
#define SERVICE_APP	  "Service.exe"
// if not SERVICE_NAME installed
// install the service.



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

bool MyCreateProcess(LPCSTR appName, LPSTR appArgs = NULL)
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

	snprintf(output, sizeof(output), "%s", appName);

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
		return false;
	}

	return true;
}


void InstallService()
{
	char cmd[MAX_PATH] = { 0 };
	char workPath[MAX_PATH] = { 0 };
	char ServicePath[MAX_PATH] = { 0 };

	GetCurrentDirectoryA(sizeof(workPath), workPath);

	if (GetServiceStatus(SERVICE_NAME) == 0)
	{
		snprintf(ServicePath, sizeof(ServicePath), "%s\\%s", workPath, SERVICE_APP);
		snprintf(cmd, sizeof(cmd), "sc create %s binpath= \"%s\" start= auto", SERVICE_NAME, ServicePath);
		MyCreateProcess(cmd);
	}

}