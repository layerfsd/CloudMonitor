// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <signal.h>
#include <time.h>

#include <Windows.h>

extern "C"
{
	VOID SetHookOn();
	VOID SetHookOff();
};


#if _WIN64
#pragma comment (lib, "HooKeyboard-64.lib")

#else 
#pragma comment (lib, "HooKeyboard.lib")

#endif
// TODO:  在此处引用程序需要的其他头文件
