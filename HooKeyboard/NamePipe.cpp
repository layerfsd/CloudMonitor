#include "NamePipe.h"
#include <iostream>
#include <cstdio>
#include <Windows.h>
#include <vector>
#include <string>
#include <queue>

using namespace std;

static bool Accept = false;
const char * pStr = "CloudMonitor";
const char * pPipeName = "\\\\.\\pipe\\CloudMonitor";
// ȫ�ֱ���,�����ܵ����
HANDLE            g_hNamedPipe = NULL;

bool CreateNamedPipeInServer()
{
	HANDLE                    hEvent;
	OVERLAPPED                ovlpd;
	//������Ҫ���������ܵ�
	//���ﴴ������˫��ģʽ��ʹ���ص�ģʽ�������ܵ�
	g_hNamedPipe = CreateNamedPipe(pPipeName,
		PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
		0, 1, 1024, 1024, 0, NULL);
	if (INVALID_HANDLE_VALUE == g_hNamedPipe)
	{
		g_hNamedPipe = NULL;
		cout << "���������ܵ�ʧ�� ..." << endl << endl;
		return false;
	}
	//����¼��Եȴ��ͻ������������ܵ�
	//���¼�Ϊ�ֶ������¼����ҳ�ʼ��״̬Ϊ���ź�״̬
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!hEvent)
	{
		cout << "�����¼�ʧ�� ..." << endl << endl;
		return false;
	}
	memset(&ovlpd, 0, sizeof(OVERLAPPED));
	//���ֶ������¼����ݸ� ovlap ����
	ovlpd.hEvent = hEvent;
	//�ȴ��ͻ�������
	if (!ConnectNamedPipe(g_hNamedPipe, &ovlpd))
	{
		if (ERROR_IO_PENDING != GetLastError())
		{
			CloseHandle(g_hNamedPipe);
			CloseHandle(hEvent);
			cout << "�ȴ��ͻ�������ʧ�� ..." << endl << endl;
			return false;
		}
	}
	//�ȴ��¼� hEvent ʧ��
	if (WAIT_FAILED == WaitForSingleObject(hEvent, INFINITE))
	{
		CloseHandle(g_hNamedPipe);
		CloseHandle(hEvent);
		cout << "�ȴ�����ʧ�� ..." << endl << endl;
		return false;
	}
	CloseHandle(hEvent);

	return true;
}


bool GetNamedPipeMessage(char* pReadBuf)
{
	DWORD   TotalBytesAvail = 0;
	bool    ret = false;

	// ����Ƿ��Ѿ����ܵ�����
	if (!Accept)
	{
		return false;
	}

	memset(pReadBuf, 0, MAX_PATH);
	PeekNamedPipe(g_hNamedPipe, pReadBuf, MAX_PATH, NULL, &TotalBytesAvail, NULL);
	if (TotalBytesAvail > 0)
	{
		//�������ܵ��ж�ȡ����
		if (!ReadFile(g_hNamedPipe, pReadBuf, MAX_PATH, NULL, NULL))
		{
			cout << "��ȡ����ʧ�� ..." << endl << endl;
			ret = false;
		}
		else
		{
			ret = true;
		}
	}

	return ret;
}


DWORD WINAPI ThreadNamePipe(LPVOID lpParam)
{
	cout << "sub thread started\n" << endl;
	CreateNamedPipeInServer();
	Accept = true;
	return 0;
}