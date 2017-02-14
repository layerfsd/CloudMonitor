// Uninstlal.cpp 
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <shellapi.h>
#include <shlobj.h>

#define ArraySize(ptr)		(sizeof(ptr) / sizeof(ptr[0]))

using namespace std;
void SelfDel();
string getusername();

bool SysRun(const char* cmd)
{
	FILE* execfd = NULL;

	printf("cmd: [%s]\n", cmd);

	execfd = _popen(cmd, "r");

	if (NULL == execfd)
	{
		return false;
	}

	return (0 == _pclose(execfd));
}

int main()
{
	const char *cmds[]{
		"sc stop CloudMonitorService",
		"taskkill /f /im CloudMonitor.exe",
		"taskkill /f /im MonitorService.exe",
		"taskkill /f /im MonitorService-64.exe",
		"sc delete CloudMonitorService",
	};

	for (int i = 0; i < ArraySize(cmds); i++)
	{
		SysRun(cmds[i]);
	}

	char buf[1000];
	int i = 1000;
	GetCurrentDirectory(1000, buf);
	std::string setdirectory;
	setdirectory.assign(buf);
	setdirectory.append("//..//");     //设置上一级目录
									   //a=a+"..//";
	SetCurrentDirectory(setdirectory.c_str());  //设置完成

	system("rd CloudMonitor /s/q"); //删除CloudMonitor下除了uninstall的所有文件

	string UserName = getusername();
	system("del /f/s/q C:\\Users\\%UserName%\\Desktop\\安全监管客户端.lnk");//删除桌面快捷方式
	
	SelfDel(); //自删除
	return 0;
}

//获取本机登录的用户名Username
string getusername()
{
	char szUser[80];
	DWORD cbUser = 80;
	if (GetUserName(szUser, &cbUser))
		return szUser;
	else
		return("GetUserName failed.\n");
}


//删除自身的函数
void SelfDel()
{
	SHELLEXECUTEINFO sei;
	TCHAR szModule[MAX_PATH], szComspec[MAX_PATH], szParams[MAX_PATH];

	// 获得自身文件名. 获取cmd的全路径文件名
	if ((GetModuleFileName(0, szModule, MAX_PATH) != 0) &&
		(GetShortPathName(szModule, szModule, MAX_PATH) != 0) &&
		(GetEnvironmentVariable("COMSPEC", szComspec, MAX_PATH) != 0))
	{
		// 设置命令参数.
		lstrcpy(szParams, "/c del ");
		lstrcat(szParams, szModule);
		lstrcat(szParams, " > nul");

		// 设置结构成员.
		sei.cbSize = sizeof(sei);
		sei.hwnd = 0;
		sei.lpVerb = "Open";
		sei.lpFile = szComspec;
		sei.lpParameters = szParams;
		sei.lpDirectory = 0;
		sei.nShow = SW_HIDE;
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;

		// 创建cmd进程.
		if (ShellExecuteEx(&sei))
		{
			// 设置cmd进程的执行级别为空闲执行,使本程序有足够的时间从内存中退出. 
			SetPriorityClass(sei.hProcess, IDLE_PRIORITY_CLASS);
			// 将自身进程的优先级置高
			SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

			// 通知Windows资源浏览器,本程序文件已经被删除.
			SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, szModule, 0);
		}
	}
}