#include "service.h"

#define DEPEND_APP_NAME			"MonitorService.exe"
#define DEPEND_APP_NAME_64		"MonitorService-64.exe"

#define BACKEND_FLAG			"--backend"



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
	char	cmd[MAX_PATH];

	// 找不到目标进程时，才启动
	if (!FindProcessPid(DEPEND_APP_NAME, dwPid))
	{
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "%s %s", DEPEND_APP_NAME, BACKEND_FLAG);

		StartInteractiveProcess(cmd, NULL);
		WriteToLog("[SERVICE START] " DEPEND_APP_NAME);
	}

	// 检测系统是否支持64位程序运行
	if (IsWow64() && !FindProcessPid(DEPEND_APP_NAME_64, dwPid))
	{
		// 本hook模块在win7 64 位上运行崩溃
		// 在win8、win10 上正常
		if (!IsWin7())
		{
			memset(cmd, 0, sizeof(cmd));
			snprintf(cmd, sizeof(cmd), "%s %s", DEPEND_APP_NAME_64, BACKEND_FLAG);
			StartInteractiveProcess(cmd, NULL);
		}
		WriteToLog("[SERVICE START] " DEPEND_APP_NAME_64);
	}
	return true;
}
