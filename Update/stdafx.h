// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
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
