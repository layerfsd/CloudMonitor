#include "stdafx.h"

#define SERVICE_NAME  "CloudMonitorService"
#define SERVICE_APP	  "Service.exe"
// if not SERVICE_NAME installed
// install the service.

void InstallService()
{
	char cmd[MAX_PATH] = { 0 };
	char workPath[MAX_PATH] = { 0 };
	char ServicePath[MAX_PATH] = { 0 };

	GetCurrentDirectoryA(sizeof(workPath), workPath);
	snprintf(ServicePath, sizeof(ServicePath), "%s\\%s", workPath, SERVICE_APP);

	snprintf(cmd, sizeof(cmd), "sc create %s binpath=\"%s\" start=delayed-auto", SERVICE_NAME,  ServicePath);
	system(cmd);
}