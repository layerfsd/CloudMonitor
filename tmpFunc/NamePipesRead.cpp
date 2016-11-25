#include "stdafx.h"

using namespace std;

//������������洴���������ܵ����
HANDLE            hNamedPipe;

const char *    pStr = "Zachary";
const char *    pPipeName = "\\\\.\\pipe\\ZacharyPipe";
HANDLE                    hEvent;
OVERLAPPED                ovlpd;


//���������ܵ�
bool CreateNamedPipeInServer();

//�������ܵ��ж�ȡ����
int NamedPipeReadInServer();



bool CreateNamedPipeInServer()
{

	//������Ҫ���������ܵ�
	//���ﴴ������˫��ģʽ��ʹ���ص�ģʽ�������ܵ�
	hNamedPipe = CreateNamedPipeA(pPipeName,
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		0, 1, 1024, 1024, 0, NULL);

	if (INVALID_HANDLE_VALUE == hNamedPipe)
	{
		hNamedPipe = NULL;
		cout << "���������ܵ�ʧ�� ..." << endl << endl;
		return false;
	}
	return true;
}


int NamedPipeReadInServer()
{
	//����¼��Եȴ��ͻ������������ܵ�
	//���¼�Ϊ�ֶ������¼����ҳ�ʼ��״̬Ϊ���ź�״̬
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!hEvent)
	{
		cout << "�����¼�ʧ�� ..." << endl << endl;
		return 1;
	}

	memset(&ovlpd, 0, sizeof(OVERLAPPED));

	//���ֶ������¼����ݸ� ovlap ����
	ovlpd.hEvent = hEvent;

	//�ȴ��ͻ�������
	if (!ConnectNamedPipe(hNamedPipe, &ovlpd))
	{
		if (ERROR_IO_PENDING != GetLastError())
		{
			CloseHandle(hNamedPipe);
			CloseHandle(hEvent);

			cout << "�ȴ��ͻ�������ʧ�� ..." << endl << endl;
			return 2;
		}
	}

	//�ȴ��¼� hEvent ʧ��
	if (WAIT_FAILED == WaitForSingleObject(hEvent, INFINITE))
	{
		CloseHandle(hNamedPipe);
		CloseHandle(hEvent);

		cout << "�ȴ�����ʧ�� ..." << endl << endl;
		return 3;
	}

	char *            pReadBuf;

	pReadBuf = new char[strlen(pStr) + 1];
	memset(pReadBuf, 0, strlen(pStr) + 1);

#if 1





	int				 RetValue = 0;

	char recvBuf[4] = { 0 };


	//�������ܵ��ж�ȡ����
	if (!ReadFile(hNamedPipe, recvBuf, 4, NULL, NULL))
	{
		return 4;
	}
	CloseHandle(hNamedPipe);
	CloseHandle(hEvent);

	RetValue = atoi(recvBuf);
	return RetValue;



	//int RetValue = 0;
	//if (!ReadFile(hNamedPipe, &RetValue, sizeof(int), NULL, NULL))
	//{
	//	return 4;
	//}
	//cout << "Get: " << RetValue << endl;

#else
	//�������ܵ��ж�ȡ����
	if (!ReadFile(hNamedPipe, pReadBuf, strlen(pStr), &dwRead, NULL))
	{
		delete[]pReadBuf;

		cout << "read error ..." << endl << endl;
		return 4;
	}
	cout << "read��    " << pReadBuf << endl << endl;
#endif
	return 0;
}



void testNamePipe()
{
	CreateNamedPipeInServer();

	//���տͻ��˷���������
	cout << "get: " << NamedPipeReadInServer()  << endl;
}
