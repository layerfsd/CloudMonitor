#pragma once
#ifndef  MY_TOOLS_INCLUDE
#define MY_TOOLS_INCLUDE

// ��������ظ�֪ͨ����
#define MAX_RETRY_TIME	15	

#include <Windows.h>


// ��ʼ�� TCP ����,������Ҫ����˫�������֤����
BOOL InitTcpConnection();


// ���̨������һ����Ϣ
VOID SendMsg2Backend();


// �ж��Ƿ�Ϊһ��"�����ļ�"
BOOL ProcessFilePath(LPCSTR lpFilePath);

#endif // ! MY_TOOLS_INCLUDE
