// filename:

#include <windows.h>
#include <Dbt.h>
#include <stdio.h>

#define UPDATE_CHECKED_FLAG	"UPDATE_CHECKED"
#define UPDATE_APP_NAME		"Update.exe"
#define UPDATE_ARGS			"--backend"

#define MASTER_DAEMON		"Daemon.exe"

#define MASTER_APP_NAME		"CloudMonitor.exe"
#define MASTER_APP_ARGS		"--autostart"


char* SERVICE_NAME = "CloudMonitorService";
DWORD SLEEP_TIME = 10;		// seconds

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void SetWorkPath();

void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);
int  InitService();

// ��¼����������־
int WriteToLog(char* str);

// ����Ƿ��Ѿ�ִ�й����Ը��¡�����
bool IsUpdateChecked();
