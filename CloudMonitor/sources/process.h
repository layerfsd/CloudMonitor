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
	char		title[MAX_PATH]; //Ϊ�������ǣ����Ը�Զ�̿����м��ϣ����̱��⣢
	DWORD		pid;

	int			seq;
	bool		status;
	bool 		shutdown;
};




//  Forward declarations:
BOOL GetProcessList(vector<Process>& plst);
bool ShowProcessList(vector<Process>& plst);

// ���ݽ������ɱ��һ������,
// shutdown ��־�����Ƿ�Ҫɱ��һ������
// KillAll Ϊ��ʱ,��ɱ�����м�ؽ���
int  KillProcess(vector<Process>& plst, bool KillAll = false);

// ��ȡ�ɹ��رս��̵����
bool GenKillResult(vector<Process>& plst, string& message);
//int  KillProcess(vector<Process>& plst, string& message, bool KillAll = false);
#endif // ! _PROCESS_H__