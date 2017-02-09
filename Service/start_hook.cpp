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
	OSVERSIONINFO osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	return ((osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion == 1));
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


inline void  Start32HookService()
{
	DWORD	dwPid;
	char	cmd[MAX_PATH];
	if (!FindProcessPid(DEPEND_APP_NAME, dwPid)) {
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "%s %s %s", MASTER_DAEMON, DEPEND_APP_NAME, BACKEND_FLAG);

		if (StartInteractiveProcess(cmd, NULL))
		{
			WriteToLog("[SERVICE-START] " DEPEND_APP_NAME " OK");
		}
		else
		{
			WriteToLog("[SERVICE-START] " DEPEND_APP_NAME " FAILEDs");
		}
	}
}

inline void  Start64HookService()
{
	DWORD	dwPid;
	char	cmd[MAX_PATH];
	if (!FindProcessPid(DEPEND_APP_NAME_64, dwPid)) {
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "%s %s %s", MASTER_DAEMON, DEPEND_APP_NAME_64, BACKEND_FLAG);

		if (StartInteractiveProcess(cmd, NULL))
		{
			WriteToLog("[SERVICE-START] " DEPEND_APP_NAME_64 " OK");
		}
		else
		{
			WriteToLog("[SERVICE-START] " DEPEND_APP_NAME_64 " FAILEDs");
		}
	}
}


bool StartHookService()
{
	if (IsWin7())
	{
		if (IsWow64())
		{
			Start64HookService();
		}
		else
		{
			Start32HookService();
		}
	}
	else
	{
		Start32HookService();
		if (IsWow64())
		{
			Start64HookService();
		}
	}
	return true;
}
