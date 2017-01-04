// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <Windows.h>
#include <tlhelp32.h>


#include <string.h>
#include <stdio.h>
#include <tchar.h>

#include <io.h>			// _access()
#include <direct.h>		// _mkdir()

#include <curl/curl.h>	// libcurl


#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;


#define ArraySize(ptr)		(sizeof(ptr) / sizeof(ptr[0]))
#define B2b(returnValue)	(returnValue) == TRUE ? true : false

#include "manage.h"
#include "CloudVersion.h"

#pragma comment(lib, "libcurl_a.lib")
