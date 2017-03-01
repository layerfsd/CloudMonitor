#pragma once
#ifndef  _PATCHES_ALBERT__
#define _PATCHES_ALBERT__
#include <Windows.h>


//#define _DEBUG

#define SERVICE_NAME			"CloudMonitorService"

#define ArraySize(ptr)	(sizeof(ptr) / sizeof(ptr[0]))
#define ErrorMsg(strs)  printf("ErrorMsg [%s] func [%s] line [%d]\n", strs, __func__, __LINE__);

#define UPDATE_CHECKED_FLAG	"UPDATE_CHECKED"

bool MyCreateProcess(LPCSTR appName, LPSTR appArgs);


void InitDir(bool hide);
bool StartHookService();	// 启动 MonitorService.exe

void SetWorkPath();

void CheckDaemonService();

int GetServiceStatus(const char* name);

bool IsServiceRunning();

// 判断用户操作系统是否为win7
bool IsWin7();

//CopyFrom: https://msdn.microsoft.com/en-us/library/windows/desktop/ms684139(v=vs.85).aspx
BOOL IsWow64();		// 检测当前系统是否支持64位程序运行

// 记录运行日志
int WriteToLog(char* str);

// 删除临时文件
void CleanTmpFiles(SFile& file);

// 调试输出函数
inline void XTrace(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#if defined(_DEBUG) || defined(SHOW_DEBUG_MESSAGES)
	vfprintf_s(stdout, fmt, args);
#endif
	va_end(args);
}

int __stdcall DoStartSvc(const char* szSvcName);

#endif // ! _PATCHES_ALBERT__
