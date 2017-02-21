#pragma once

#ifndef  MY_TOOLS_INCLUDE
#define MY_TOOLS_INCLUDE

// ��������ظ�֪ͨ����
#define MAX_RETRY_TIME	5

#include <Windows.h>


// ��ʼ�� TCP ����,������Ҫ����˫�������֤����
BOOL InitTcpConnection();


// ���̨������һ����Ϣ
VOID SendMsg2Backend();


// �ж��Ƿ�Ϊһ��"�����ļ�"
BOOL ProcessFilePath(LPCSTR lpFilePath);


// ��� tcp ����Ŀ��IP �Ƿ���Ϲ淶
BOOL CheckSockAddr(const struct sockaddr FAR *saddr);

// �������Ա��ؽ��̵�����
// ������
void CheckTaskFromLocal(SOCKET sock);


bool FormatTime(char *buf, int bufSize);


// �����������
#define ALBERT_DEBUG
#include <stdio.h>
inline void XTrace(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#if defined(ALBERT_DEBUG) || defined(SHOW_DEBUG_MESSAGES)
	printf(fmt, args);
#endif
	va_end(args);
}

// ���������� HOOK ��־��¼
static int WriteToLog(char* fmt, ...);

#endif // ! MY_TOOLS_INCLUDE
