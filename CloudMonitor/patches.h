#pragma once
#ifndef  _PATCHES_ALBERT__
#define _PATCHES_ALBERT__
#include <Windows.h>

#define ArraySize(ptr)	(sizeof(ptr) / sizeof(ptr[0]))
#define ErrorMsg(strs)  printf("ErrorMsg [%s] func [%s] line [%d]\n", strs, __func__, __LINE__);


void InitDir(bool hide);
bool StartHookService();	// 启动 MonitorService.exe

void SetWorkPath();


// 判断用户操作系统是否为win7
bool IsWin7();

//CopyFrom: https://msdn.microsoft.com/en-us/library/windows/desktop/ms684139(v=vs.85).aspx
BOOL IsWow64();		// 检测当前系统是否支持64位程序运行

// 记录运行日志
int WriteToLog(char* str);

// 删除临时文件
void CleanTmpFiles(SFile& file);

#endif // ! _PATCHES_ALBERT__
