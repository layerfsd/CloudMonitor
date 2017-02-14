// Uninstlal.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

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
    return 0;
}

