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
// 全局变量,命名管道句柄
HANDLE            g_hNamedPipe = NULL;

bool CreateNamedPipeInServer()
{
	HANDLE                    hEvent;
	OVERLAPPED                ovlpd;
	//首先需要创建命名管道
	//这里创建的是双向模式且使用重叠模式的命名管道
	g_hNamedPipe = CreateNamedPipe(pPipeName,
		PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
		0, 1, 1024, 1024, 0, NULL);
	if (INVALID_HANDLE_VALUE == g_hNamedPipe)
	{
		g_hNamedPipe = NULL;
		cout << "创建命名管道失败 ..." << endl << endl;
		return false;
	}
	//添加事件以等待客户端连接命名管道
	//该事件为手动重置事件，且初始化状态为无信号状态
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!hEvent)
	{
		cout << "创建事件失败 ..." << endl << endl;
		return false;
	}
	memset(&ovlpd, 0, sizeof(OVERLAPPED));
	//将手动重置事件传递给 ovlap 参数
	ovlpd.hEvent = hEvent;
	//等待客户端连接
	if (!ConnectNamedPipe(g_hNamedPipe, &ovlpd))
	{
		if (ERROR_IO_PENDING != GetLastError())
		{
			CloseHandle(g_hNamedPipe);
			CloseHandle(hEvent);
			cout << "等待客户端连接失败 ..." << endl << endl;
			return false;
		}
	}
	//等待事件 hEvent 失败
	if (WAIT_FAILED == WaitForSingleObject(hEvent, INFINITE))
	{
		CloseHandle(g_hNamedPipe);
		CloseHandle(hEvent);
		cout << "等待对象失败 ..." << endl << endl;
		return false;
	}
	CloseHandle(hEvent);

	return true;
}


bool GetNamedPipeMessage(char* pReadBuf)
{
	DWORD   TotalBytesAvail = 0;
	bool    ret = false;

	// 检测是否已经接受到连接
	if (!Accept)
	{
		return false;
	}

	memset(pReadBuf, 0, MAX_PATH);
	PeekNamedPipe(g_hNamedPipe, pReadBuf, MAX_PATH, NULL, &TotalBytesAvail, NULL);
	if (TotalBytesAvail > 0)
	{
		//从命名管道中读取数据
		if (!ReadFile(g_hNamedPipe, pReadBuf, MAX_PATH, NULL, NULL))
		{
			cout << "读取数据失败 ..." << endl << endl;
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