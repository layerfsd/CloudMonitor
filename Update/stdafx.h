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

#include <curl/curl.h>


#include <iostream>
#include <vector>



using namespace std;

#define ArraySize(ptr)	(sizeof(ptr) / sizeof(ptr[0]))


#include "manage.h"
#include "CloudVersion.h"

#pragma comment(lib, "libcurl_a.lib")
