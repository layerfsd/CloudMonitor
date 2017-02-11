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

#endif // ! MY_TOOLS_INCLUDE
