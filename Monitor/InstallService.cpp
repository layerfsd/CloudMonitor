#include "stdafx.h"

#define SERVICE_NAME  "CloudMonitorService"
#define SERVICE_APP	  "Service.exe"
// if not SERVICE_NAME installed
// install the service.



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
void InstallService()
{
	char cmd[MAX_PATH] = { 0 };
	char workPath[MAX_PATH] = { 0 };
	char ServicePath[MAX_PATH] = { 0 };

	GetCurrentDirectoryA(sizeof(workPath), workPath);
	snprintf(ServicePath, sizeof(ServicePath), "%s\\%s", workPath, SERVICE_APP);


	snprintf(cmd, sizeof(cmd), "sc create %s binpath= \"%s\" start= delayed-auto", SERVICE_NAME, ServicePath);
	SysRun(cmd);
}