#include "tools.h"
#include "ILHook.h"
#include <comdef.h>
#include <stdio.h>
#include <Windows.h>

#include <iostream>
#include <string>
#include <queue>

using namespace std;

#pragma data_seg("Shared")
HHOOK g_hHook2= NULL;
HHOOK g_hHook = NULL;
HWND  g_ExeHwnd = NULL;
#pragma data_seg()
#pragma comment (linker, "/section:Shared,RWS")

HINSTANCE g_hInst;


extern "C" __declspec(dllexport) VOID SetHookOn();
extern "C" __declspec(dllexport) VOID SetHookOff();

CILHook CreateFileHook;
CILHook CreateProcessHook;
CILHook connectHook;

DWORD WINAPI ThreadProc(LPVOID lpParam);


LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(g_hHook, code, wParam, lParam);
}


VOID SetHookOn()
{
	HANDLE hThrd = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// 创建一个本地 TCP 端口,发送敏感事件

	//g_hHook = SetWindowsHookEx(WH_CALLWNDPROC, GetMsgProc, g_hInst, 0);	// 窗口函数的过滤函数， 无法对wps打开多个文档时hook
	g_hHook2 = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hInst, 0);		// 获取消息过滤函数，无法对wps新建进程时hook
	//g_hHook = SetWindowsHookEx(WH_KEYBOARD, GetMsgProc, g_hInst, 0);		// 键盘过滤，系统崩溃

	// 消息过滤函数,在64为hook中，只要转到文件目录，则自动上场了，甚至没有打开操作
	// 同时，对于wps 仍然无效
	//g_hHook = SetWindowsHookEx(WH_MSGFILTER, GetMsgProc, g_hInst, 0);		
	//g_hHook = SetWindowsHookEx(WH_CALLWNDPROCRET, GetMsgProc, g_hInst, 0);

	//截获发向外壳应用程序的消息, 谢天谢地，这个成功了
	// g_hHook = SetWindowsHookEx(WH_SHELL, GetMsgProc, g_hInst, 0);			
	WaitForSingleObject(hThrd, INFINITE);
}

extern BOOL KEEP_RUNNING;
VOID SetHookOff()
{
	// 卸载钩子
	if (NULL != g_hHook)
		UnhookWindowsHookEx(g_hHook);
	if (NULL != g_hHook2)
		UnhookWindowsHookEx(g_hHook2);
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
	CHAR tp[1024] = { 0 };

	int w_nlen = WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, NULL, 0, NULL, false);
	WideCharToMultiByte(CP_ACP, 0, lpFileName, -1, tp, w_nlen, NULL, false);

	ProcessFilePath(tp);
	//printf("[CHECKING] %s\n", tp);
	CreateFileHook.UnHook();

	HANDLE hRet = CreateFileW(
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



int 
WINAPI 
Myconnect(
	SOCKET s,
	const struct sockaddr FAR *saddr,
	int namelen)
{
	LONG iRet = FALSE;
	connectHook.UnHook();

	// 当程序开启‘断开网络功能后’，
	// 仅目的地址为本地局域网地址和客户端服务器的地址时，才允许建立TCP连接
	// 否则不允许建立 tcp 连接
	if (CheckSockAddr(saddr))
	{
		iRet = connect(s, saddr, namelen);
	}
	else
	{
		// 决定好不允许建立当前 tcp 连接后
		// 直接给调用者返回 -1
		iRet = WSAENETUNREACH;
	}

	connectHook.ReHook();
	return iRet;

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
		connectHook.Hook("ws2_32.dll", "connect", (PROC)Myconnect);

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