// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
extern "C"
{
	VOID SetHookOn();
	VOID SetHookOff();
};

#pragma comment (lib, "HooKeyboard.lib")

// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
