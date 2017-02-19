// Update.cpp : �������̨Ӧ�ó������ڵ㡣
//�����������ˡ��޷��������ⲿ���š��Ĵ����������������ҵ��˽������--->�ڡ�Ԥ�����м���������
// http://www.cnblogs.com/ytjjyy/archive/2012/05/19/2508803.html

#include "stdafx.h"

bool MyCreateProcess(LPCSTR appName, LPSTR appArgs = NULL)
{
	STARTUPINFOA   StartupInfo;		//���������������Ϣ�ṹ����    
	PROCESS_INFORMATION pi;
	char output[MAXBYTE];

	if (NULL == appName)
	{
		return false;
	}

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	snprintf(output, sizeof(output), "%s", appName);

	if (CreateProcessA(NULL,
		output,
		NULL,
		NULL,
		FALSE,
		//0,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&StartupInfo,
		&pi))
	{

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		snprintf(output, MAXBYTE, "[ERROR] CreateProcess: %s", appName);
		return false;
	}

	return true;
}

const char *cmds[]{
	"taskkill /f /im CloudMonitor.exe",
	"taskkill /f /im MonitorService.exe",
	"taskkill /f /im MonitorService-64.exe",
};



int main(int argc, char *argv[])
{
	CloudVersion ver;
	
	// �ȹر����ڹ����Ľ��̣��Է�ֹ�滻ʱ��������������ʱ����ʧ��
	for (int i = 0; i < ArraySize(cmds); i++)
	{
		MyCreateProcess(cmds[i]);
	}


	// ��ȡ��ǰ����İ汾��
	cout << "Current Version: " << ver.GetCurVersion() << endl;

	if (2 == argc && 0 == strncmp(UPDATE_ARGS, argv[1], sizeof(UPDATE_ARGS)))
	{
		EnableLog();
	}

	// ��ȡ����˱�������°汾��
	if (!ver.GetLatestVersion())
	{
		
		WriteToLog("GetLatestVersion() FAILED\n");
		printf("GetLatestVersion() FAILED\n");
		return 1;
	}
	WriteToLog("GetLatestVersion Ok");


	if (!ver.WhetherUpdate())
	{
		printf("Already Latest Version.\n");
		WriteToLog("Already Latest Version.\n");
		return 0;
	}
	WriteToLog("Begin Fetch HashList");


	// ��ȡ���°汾���ļ���ϣ�б�
	if (!ver.RequestHashList())
	{
		printf("Request Latest HashList failed.\n");
		WriteToLog("Request Latest HashList failed");
		return 1;
	}
	WriteToLog("Request Latest HashList Ok");


	// �����ļ���ϣ��ͬ����ȡ���С����¡����ļ�
	if (!ver.DownloadLatestFiles(TMPDOWN_DIR))
	{
		printf("Download Latest Files failed.\n");
		WriteToLog("Download Latest Files failed");
		return 1;
	}
	WriteToLog("Download Latest Files Ok");


	// ����ʱĿ¼�е��ļ��滻��װ��Ŀ¼���ļ�
	if (!ver.ReplaceFiles(TMPDOWN_DIR))
	{ 
		printf("Replace Files failed.\n");
		WriteToLog("Replace Files failed");
		ver.RollBack();
		return 1;
	}
	WriteToLog("Replace Files Ok");
	
	return 0;
}
