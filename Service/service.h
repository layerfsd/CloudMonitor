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

#define LOGFILE_MAX_SIZE	(1024 * 1024 * 3)	// 日志文件最大值为3MB

#define SLEEP_TIME 2500		// seconds

void SetWorkPath();

bool StartInteractiveProcess(LPTSTR cmd, LPCTSTR cmdDir);

void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);
int  InitService();

// 记录服务运行日志
int WriteToLog(char* str);

// 检查是否已经执行过“自更新”程序
bool IsUpdateChecked();

bool MyCreateProcess(LPCSTR appName, LPSTR appArgs = NULL);

BOOL FindProcessPid(LPCSTR ProcessName, DWORD& dwPid);