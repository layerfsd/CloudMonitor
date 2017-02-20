// Update.cpp : �������̨Ӧ�ó������ڵ㡣
//�����������ˡ��޷��������ⲿ���š��Ĵ����������������ҵ��˽������--->�ڡ�Ԥ�����м���������
// http://www.cnblogs.com/ytjjyy/archive/2012/05/19/2508803.html

#include "stdafx.h"

int main(int argc, char *argv[])
{
	CloudVersion ver;
	
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
