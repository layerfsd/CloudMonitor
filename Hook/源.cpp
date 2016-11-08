#include <Windows.h>
#include <iostream>

using namespace std;
//用来保存在客户端通过 CreateFile 打开的命名管道句柄


const char * pStr = "CloudMonitor";
const char * pPipeName = "\\\\.\\pipe\\CloudMonitor";
//打开命名管道
bool OpenNamedPipeInClient(HANDLE& hNamedPipe);


//客户端往命名管道中写入数据
bool NamedPipeWriteInClient(HANDLE& hNamedPipe, const char *buf);
#include <stdio.h>

int main(int argc, char *argv[])
{
	//char tmp[MAX_PATH] = "F:\\NutStore\\SSL传输\\安全办公信息监控平台项目研发方案20160922.docx";
	char path[MAX_PATH];
	char cmd[MAX_PATH] = "FileMon.exe wps";
	int proPid = 8964;

	HANDLE            hNamedPipe;
	OpenNamedPipeInClient(hNamedPipe);


	FILE   *pPipe;

	snprintf(cmd, MAX_PATH, "FileMon.exe %d",  proPid);
	//fputs(cmd, stdout);
	if ((pPipe = _popen(cmd, "r")) == NULL)
	{
		return -1;
	}

	while (true)
	{
		memset(path, 0, sizeof(path));
		if (!fgets(path, MAX_PATH, pPipe))
		{
			printf(path);
		}
	}

	_pclose(pPipe);

		//往命名管道中写入数据
		//NamedPipeWriteInClient(hNamedPipe, tmp);

	return 0;
}


bool OpenNamedPipeInClient(HANDLE& hNamedPipe)
{
	//等待连接命名管道
	if (!WaitNamedPipe(pPipeName, NMPWAIT_WAIT_FOREVER))
	{
		cout << "命名管道实例不存在 ..." << endl << endl;
		return false;
	}
	//打开命名管道
	hNamedPipe = CreateFile(pPipeName, GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hNamedPipe)
	{
		cout << "打开命名管道失败 ..." << endl << endl;
		return false;
	}

	return true;
}


bool NamedPipeWriteInClient(HANDLE& hNamedPipe, const char *buf)
{
	DWORD                dwWrite;
	//向命名管道中写入数据
	if (!WriteFile(hNamedPipe, buf, strlen(buf), &dwWrite, NULL))
	{
		cout << "写入数据失败 ..." << endl << endl;
		return false;
	}

	return true;
}
