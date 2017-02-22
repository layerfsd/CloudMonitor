// filename:

#include <windows.h>
#include <Dbt.h>
#include <stdio.h>

#define SERVICE_NAME		"CloudMonitorService"

#define UPDATE_CHECKED_FLAG	"UPDATE_CHECKED"
#define UPDATE_APP_NAME		"Update.exe"
#define UPDATE_ARGS			"--backend"

#define MASTER_DAEMON		"Daemon.exe"

#define MASTER_APP_NAME		"CloudMonitor.exe"
#define MASTER_APP_ARGS		"--start"

#define LOG_FILE_PATH		"Service.txt"

#define LOGFILE_MAX_SIZE	(1024 * 1024 * 3)	// ��־�ļ����ֵΪ3MB

#define SLEEP_TIME 2500		// seconds

void SetWorkPath();

bool StartInteractiveProcess(LPTSTR cmd, LPCTSTR cmdDir);

void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);
int  InitService();

// ��¼����������־
int WriteToLog(char* str);

// ����Ƿ��Ѿ�ִ�й����Ը��¡�����
bool IsUpdateChecked();

bool MyCreateProcess(LPCSTR appName, LPSTR appArgs = NULL);

BOOL FindProcessPid(LPCSTR ProcessName, DWORD& dwPid);