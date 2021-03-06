#pragma once
#ifndef _PROCESS_H__ALBERTOFWB
#define _PROCESS_H__ALBERTOFWB

//copy from: https://msdn.microsoft.com/en-us/library/windows/desktop/ms686701(v=vs.85).aspx

#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <vector>

using namespace std;

#define FILE_NETEDIT    '1'
#define FILE_COPY2USB	'2'



enum PStatus
{
	CLOSED = 0,
	RUNNING
};
struct Process
{
	char		name[MAX_PATH];
	int			code;
	char		title[MAX_PATH]; //为升级考虑，可以给远程控制中加上＂进程标题＂
	DWORD		pid;

	int			seq;
	bool		status;
	bool 		shutdown;
};

// 检查是否有常用网络进程在运行,包括 (QQ, 浏览器)
// 如果存在,则记录到 logMsg 中
bool CheckNetworkApps(vector<Process>& plst, string& logMsg);

// 远程控制接口
bool RemoteGetProcessList(string& message, string& args);
bool RemoteKillProcess(string& message, string& args);


//  Forward declarations:
BOOL GetProcessList(vector<Process>& plst);
bool ShowProcessList(vector<Process>& plst);

// 根据进程序号杀死一个进程,
// shutdown 标志决定是否要杀掉一个进程
// KillAll 为真时,则杀掉所有监控进程
int  KillProcess(vector<Process>& plst, bool KillAll = false);

// 获取成功关闭进程的序号
bool GenKillResult(vector<Process>& plst, string& message);



BOOL FindProcessPid(LPCSTR ProcessName, DWORD& dwPid);

//int  KillProcess(vector<Process>& plst, string& message, bool KillAll = false);
#endif // ! _PROCESS_H__