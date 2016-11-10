
// Use signal to attach a signal handler to the abort routine
#include "headers\FileMon.h"
#include "patches.h"
#include <signal.h>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <io.h>

using namespace std;

extern BOOL g_RUNNING;

void SignalHandler(int signal)
{
	//system("cls");
	printf("\nExciting...\n");
	g_RUNNING = FALSE;
	exit(signal);
	return;
}


void regSINGINT()
{
	typedef void(*SignalHandlerPointer)(int);

	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGINT, SignalHandler);

	return;
}

BOOL IsDirectory(const char *pDir)
{
	char szCurPath[500];
	ZeroMemory(szCurPath, 500);
	sprintf_s(szCurPath, 500, "%s//*", pDir);
	WIN32_FIND_DATAA FindFileData;
	ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));

	HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData); /**< find first file by given path. */

	if (hFile == INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		return FALSE; /** 如果不能找到第一个文件，那么没有目录 */
	}
	else
	{
		FindClose(hFile);
		return TRUE;
	}

}

BOOL DeleteDirectory(const char * DirName)
{
	//	CFileFind tempFind;		//声明一个CFileFind类变量，以用来搜索
	char szCurPath[MAX_PATH];		//用于定义搜索格式
	_snprintf(szCurPath, MAX_PATH, "%s//*.*", DirName);	//匹配格式为*.*,即该目录下的所有文件
	WIN32_FIND_DATAA FindFileData;
	ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));
	HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData);
	BOOL IsFinded = TRUE;
	while (IsFinded)
	{
		IsFinded = FindNextFileA(hFile, &FindFileData);	//递归搜索其他的文件
		if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, "..")) //如果不是"." ".."目录
		{
			string strFileName = "";
			strFileName = DirName;
			strFileName += FindFileData.cFileName;
			DeleteFileA(strFileName.c_str());
		}
	}
	FindClose(hFile);

	return TRUE;
}


void InitDir()
{
	regSINGINT(); //注册 CTRL+C 信号处理函,正常终止会话.


	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "\\..\\");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectory(strModule);
	GetCurrentDirectory(sizeof(strModule), strModule);



	// 创建临时目录
	if (-1 == _access(TMP_DIR, 0))
	{
		cout << TMP_DIR << " not exists!!!" << endl;
		cout << "mkdir: " << TMP_DIR << endl;
		_mkdir(TMP_DIR);
	}
	else
	{
		cout << "Working Dir: " << strModule << endl;
		DeleteDirectory(TMP_DIR);	// 程序每次启动时,清空所有临时文件
	}
	return;
}