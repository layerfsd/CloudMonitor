#include "stdafx.h"

//  Forward declarations:
BOOL GetProcessList();
BOOL ListProcessModules(DWORD dwPID);
BOOL ListProcessThreads(DWORD dwOwnerPID);
void printError(TCHAR* msg);

//int main(void)
//{
//	GetProcessList();
//	return 0;
//}

BOOL GetProcessList()
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		printError(TEXT("Process32First")); // show cause of failure
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		//_tprintf(TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile);

		// Retrieve the priority class.
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if (hProcess != NULL)
		{
			HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
			MODULEENTRY32 me32;

			// Take a snapshot of all modules in the specified process.
			hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);
			if (hModuleSnap == INVALID_HANDLE_VALUE)
			{
				return(FALSE);
			}

			// Set the size of the structure before using it.
			me32.dwSize = sizeof(MODULEENTRY32);

			// Retrieve information about the first module,
			// and exit if unsuccessful
			if (!Module32First(hModuleSnap, &me32))
			{
				CloseHandle(hModuleSnap);           // clean the snapshot object
				return(FALSE);
			}

			// Now walk the module list of the process,
			// and display information about each module
			do
			{
				if (!wcscmp(TEXT("HooKeyboard.dll"), me32.szModule))
				{
					_tprintf(TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile);
					_tprintf(TEXT("\n     MODULE NAME:     %s"), me32.szModule);
					_tprintf(TEXT("\n     Executable     = %s"), me32.szExePath);
					TerminateProcess(hProcess, 1);
					break;	//找到包含此模块的进程, 干掉此进程
				}
			} while (Module32Next(hModuleSnap, &me32));
			if (NULL != hModuleSnap)
			{
				CloseHandle(hModuleSnap);
			}
			CloseHandle(hProcess);
		}

		//_tprintf(TEXT("\n  Process ID        = 0x%08X"), pe32.th32ProcessID);
		//_tprintf(TEXT("\n  Thread count      = %d"), pe32.cntThreads);
		//_tprintf(TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID);
		//_tprintf(TEXT("\n  Priority base     = %d"), pe32.pcPriClassBase);
		//if (dwPriorityClass)
		//	_tprintf(TEXT("\n  Priority class    = %d"), dwPriorityClass);

		// List the modules and threads associated with this process
		ListProcessModules(pe32.th32ProcessID);

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return(TRUE);
}


BOOL ListProcessModules(DWORD dwPID)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		return(FALSE);
	}

	// Set the size of the structure before using it.
	me32.dwSize = sizeof(MODULEENTRY32);

	// Retrieve information about the first module,
	// and exit if unsuccessful
	if (!Module32First(hModuleSnap, &me32))
	{
		CloseHandle(hModuleSnap);           // clean the snapshot object
		return(FALSE);
	}

	// Now walk the module list of the process,
	// and display information about each module
	do
	{
		if (!wcscmp(TEXT("HooKeyboard.dll"), me32.szModule))
		{
			_tprintf(TEXT("\n\n     MODULE NAME:     %s"), me32.szModule);
			_tprintf(TEXT("\n     Executable     = %s"), me32.szExePath);
		}
	} while (Module32Next(hModuleSnap, &me32));

	CloseHandle(hModuleSnap);
	printf("Done ...\n");
	getchar();
	return(TRUE);
}

void printError(TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));

	// Display the message
	_tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}
