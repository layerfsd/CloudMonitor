#include "stdafx.h"

using namespace std;

//服务端用来保存创建的命名管道句柄
HANDLE            hNamedPipe;

const char *    pStr = "Zachary";
const char *    pPipeName = "\\\\.\\pipe\\ZacharyPipe";
HANDLE                    hEvent;
OVERLAPPED                ovlpd;


//创建命名管道
bool CreateNamedPipeInServer();

//从命名管道中读取数据
int NamedPipeReadInServer();



bool CreateNamedPipeInServer()
{

	//首先需要创建命名管道
	//这里创建的是双向模式且使用重叠模式的命名管道
	hNamedPipe = CreateNamedPipeA(pPipeName,
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		0, 1, 1024, 1024, 0, NULL);

	if (INVALID_HANDLE_VALUE == hNamedPipe)
	{
		hNamedPipe = NULL;
		cout << "创建命名管道失败 ..." << endl << endl;
		return false;
	}
	return true;
}


int NamedPipeReadInServer()
{
	//添加事件以等待客户端连接命名管道
	//该事件为手动重置事件，且初始化状态为无信号状态
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!hEvent)
	{
		cout << "创建事件失败 ..." << endl << endl;
		return 1;
	}

	memset(&ovlpd, 0, sizeof(OVERLAPPED));

	//将手动重置事件传递给 ovlap 参数
	ovlpd.hEvent = hEvent;

	//等待客户端连接
	if (!ConnectNamedPipe(hNamedPipe, &ovlpd))
	{
		if (ERROR_IO_PENDING != GetLastError())
		{
			CloseHandle(hNamedPipe);
			CloseHandle(hEvent);

			cout << "等待客户端连接失败 ..." << endl << endl;
			return 2;
		}
	}

	//等待事件 hEvent 失败
	if (WAIT_FAILED == WaitForSingleObject(hEvent, INFINITE))
	{
		CloseHandle(hNamedPipe);
		CloseHandle(hEvent);

		cout << "等待对象失败 ..." << endl << endl;
		return 3;
	}

	char *            pReadBuf;

	pReadBuf = new char[strlen(pStr) + 1];
	memset(pReadBuf, 0, strlen(pStr) + 1);

#if 1





	int				 RetValue = 0;

	char recvBuf[4] = { 0 };


	//从命名管道中读取数据
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
	//从命名管道中读取数据
	if (!ReadFile(hNamedPipe, pReadBuf, strlen(pStr), &dwRead, NULL))
	{
		delete[]pReadBuf;

		cout << "read error ..." << endl << endl;
		return 4;
	}
	cout << "read：    " << pReadBuf << endl << endl;
#endif
	return 0;
}



void testNamePipe()
{
	CreateNamedPipeInServer();

	//接收客户端发来的数据
	cout << "get: " << NamedPipeReadInServer()  << endl;
}
