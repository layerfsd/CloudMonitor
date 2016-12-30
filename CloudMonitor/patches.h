#pragma once
#ifndef  _PATCHES_ALBERT__
#define _PATCHES_ALBERT__
#include <Windows.h>

#define ArraySize(ptr)	(sizeof(ptr) / sizeof(ptr[0]))
#define ErrorMsg(strs)  printf("ErrorMsg [%s] func [%s] line [%d]\n", strs, __func__, __LINE__);


void InitDir(bool hide);
bool StartHookService();	// Æô¶¯ MonitorService.exe

#endif // ! _PATCHES_ALBERT__
