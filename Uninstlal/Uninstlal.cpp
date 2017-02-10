// Uninstlal.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <string>

#define ArraySize(ptr)		(sizeof(ptr) / sizeof(ptr[0]))

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
	char buffer[1000];
	int i = 1000;
	GetCurrentDirectory(1000, buf);
	//printf("the current dictory is:%s\n", buf);
	std::string setdirectory;
	setdirectory.assign(buf);
	//printf("the dictory is :%s\n",setdirectory);
	setdirectory.append("//..//");     //设置为当前工作路径为当时的上一级
							//a=a+"..//";
	SetCurrentDirectory(setdirectory.c_str());  //设置
	//GetCurrentDirectory(1000, buffer);

	system("rd CloudMonitor /s/q");

	return 0;
}