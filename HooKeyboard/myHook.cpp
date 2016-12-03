#include "tools.h"
#include "ILHook.h"
#include <comdef.h>
#include <stdio.h>
#include <Windows.h>

#include <iostream>
#include <string>
#include <queue>

using namespace std;

//#pragma data_seg("Shared")
//queue<string> g_PathList;
//#pragma data_seg()
//#pragma comment (linker, "/section:Shared,RWS")

HINSTANCE g_hInst;
HHOOK g_hHook = NULL;
HWND  g_ExeHwnd = NULL;


extern "C" __declspec(dllexport) VOID SetHookOn();
extern "C" __declspec(dllexport) VOID SetHookOff();

CILHook CreateFileHook;
CILHook CreateProcessHook;


DWORD WINAPI ThreadProc(LPVOID lpParam);


LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(g_hHook, code, wParam, lParam);
}


VOID SetHookOn()
{
	//g_hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hInst, 0);
	HANDLE hThrd = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// 创建一个本地 TCP 端口,发送敏感事件
	g_hHook = SetWindowsHookEx(WH_CBT, GetMsgProc, g_hInst, 0);
	WaitForSingleObject(hThrd, INFINITE);
}

extern BOOL KEEP_RUNNING;
VOID SetHookOff()
{
	// 卸载钩子
	UnhookWindowsHookEx(g_hHook);
	g_hHook = NULL;
	printf("KEEP_RUNNING %d:\n", KEEP_RUNNING);
	KEEP_RUNNING = FALSE;
	printf("KEEP_RUNNING %d:\n", KEEP_RUNNING);
}


HANDLE 
WINAPI 
MyCreateFileW(
	LPCWSTR					lpFileName,
	DWORD					dwDesiredAccess,
	DWORD					dwShareMode,
	LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
	DWORD					dwCreateionDisposiotion,
	DWORD					dwFlagsAndAttributes,
	HANDLE					hTemplateFile)
{
	_bstr_t b(lpFileName);
	//ProcessFilePath(b);

	HANDLE hRet = NULL;

	//ProcessFilePath(b);

	//if (bRet)
	//	MessageBox(NULL, b, "打开文件", MB_OK);

	CreateFileHook.UnHook();

	hRet = CreateFileW(
		lpFileName,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreateionDisposiotion,
		dwFlagsAndAttributes,
		hTemplateFile);

	CreateFileHook.ReHook();
	return hRet;
}


BOOL 
WINAPI
MyCreateProcessW(
	LPCWSTR					lpApplicationName,
	LPWSTR					lpCommandLine,
	LPSECURITY_ATTRIBUTES	lpProcessAttributes,
	LPSECURITY_ATTRIBUTES	lpThreadAttributes,
	BOOL					bInheritHandles,
	DWORD					dwCreationFlags,
	LPVOID					lpEnvironment,
	LPCWSTR					lpCurrentDirectory,
	LPSTARTUPINFOW			lpStartupInfo,
	LPPROCESS_INFORMATION	lpProcessInformation
)
{
	//SetCache("MyCreateProcessW\n");

	//ProcessFilePath(FileName);
	_bstr_t b(lpCommandLine);
	//string fk = b;
	//SetCache(fk.c_str());
	MessageBox(NULL, b, "新进程", MB_OK);

	BOOL bRet = FALSE;
	CreateProcessHook.UnHook();
	bRet = CreateProcessW(
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation);
	CreateProcessHook.ReHook();
	return bRet;
}



BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID loReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = (HINSTANCE)hModule;

		// Install the hook
		CreateFileHook.Hook("kernel32.dll", "CreateFileW", (PROC)MyCreateFileW);
		//CreateProcessHook.Hook("kernel32.dll", "CreateProcessW", (PROC)MyCreateProcessW);

		break;

	case DLL_PROCESS_DETACH:
		CreateFileHook.UnHook();
		if (g_hHook != NULL)
		{
			SetHookOff();
		}
		break;
	}


	return TRUE;
}