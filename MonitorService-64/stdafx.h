// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
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
// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
