// filename:

#include <windows.h>
#include <Dbt.h>
#include <stdio.h>


#define MASTER_APP_NAME		"CloudMonitor.exe"


char* SERVICE_NAME = "CloudMonitorService";
DWORD SLEEP_TIME = 10;		// seconds

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void SetWorkPath();
bool StartMyService();

void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);
int  InitService();

