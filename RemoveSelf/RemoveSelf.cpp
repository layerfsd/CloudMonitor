// Uninstlal.cpp 
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <shellapi.h>
#include <shlobj.h>

#define ArraySize(ptr)		(sizeof(ptr) / sizeof(ptr[0]))

using namespace std;
void SelfDel();
string getusername();

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

	char buf[1000];
	int i = 1000;
	GetCurrentDirectory(1000, buf);
	std::string setdirectory;
	setdirectory.assign(buf);
	setdirectory.append("//..//");     //������һ��Ŀ¼
									   //a=a+"..//";
	SetCurrentDirectory(setdirectory.c_str());  //�������

	system("rd CloudMonitor /s/q"); //ɾ��CloudMonitor�³���uninstall�������ļ�

	string UserName = getusername();
	system("del /f/s/q C:\\Users\\%UserName%\\Desktop\\��ȫ��ܿͻ���.lnk");//ɾ�������ݷ�ʽ
	
	SelfDel(); //��ɾ��
	return 0;
}

//��ȡ������¼���û���Username
string getusername()
{
	char szUser[80];
	DWORD cbUser = 80;
	if (GetUserName(szUser, &cbUser))
		return szUser;
	else
		return("GetUserName failed.\n");
}


//ɾ������ĺ���
void SelfDel()
{
	SHELLEXECUTEINFO sei;
	TCHAR szModule[MAX_PATH], szComspec[MAX_PATH], szParams[MAX_PATH];

	// ��������ļ���. ��ȡcmd��ȫ·���ļ���
	if ((GetModuleFileName(0, szModule, MAX_PATH) != 0) &&
		(GetShortPathName(szModule, szModule, MAX_PATH) != 0) &&
		(GetEnvironmentVariable("COMSPEC", szComspec, MAX_PATH) != 0))
	{
		// �����������.
		lstrcpy(szParams, "/c del ");
		lstrcat(szParams, szModule);
		lstrcat(szParams, " > nul");

		// ���ýṹ��Ա.
		sei.cbSize = sizeof(sei);
		sei.hwnd = 0;
		sei.lpVerb = "Open";
		sei.lpFile = szComspec;
		sei.lpParameters = szParams;
		sei.lpDirectory = 0;
		sei.nShow = SW_HIDE;
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;

		// ����cmd����.
		if (ShellExecuteEx(&sei))
		{
			// ����cmd���̵�ִ�м���Ϊ����ִ��,ʹ���������㹻��ʱ����ڴ����˳�. 
			SetPriorityClass(sei.hProcess, IDLE_PRIORITY_CLASS);
			// ��������̵����ȼ��ø�
			SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

			// ֪ͨWindows��Դ�����,�������ļ��Ѿ���ɾ��.
			SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, szModule, 0);
		}
	}
}