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


enum PStatus
{
	CLOSED = 0,
	RUNNING
};
struct Process
{
	char		name[MAX_PATH];
	char		title[MAX_PATH]; //为升级考虑，可以给远程控制中加上＂进程标题＂
	DWORD		pid;

	int			seq;
	bool		status;
	bool 		shutdown;
};




//  Forward declarations:
BOOL GetProcessList(vector<Process>& plst);
bool ShowProcessList(vector<Process>& plst);
int  KillProcess(vector<Process>& plst, vector<int>& klst);
#endif // ! _PROCESS_H__