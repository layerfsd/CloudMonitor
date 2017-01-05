// Update.cpp : �������̨Ӧ�ó������ڵ㡣
//�����������ˡ��޷��������ⲿ���š��Ĵ����������������ҵ��˽������--->�ڡ�Ԥ�����м���������
// http://www.cnblogs.com/ytjjyy/archive/2012/05/19/2508803.html

#include "stdafx.h"


int main()
{
	//StopMyService();
	//bool bRet = GetFilesList("output.txt");

	CloudVersion ver;

	// ��ȡ��ǰ����İ汾��
	cout << "Current Version: " << ver.GetCurVersion() << endl;

	// ��ȡ����˱�������°汾��
	if (!ver.GetLatestVersion())
	{
		printf("GetLatestVersion() FAILED\n");
		return 1;
	}

	if (!ver.WhetherUpdate())
	{
		printf("Already Latest Version.\n");
		return 0;
	}

	// ��ȡ���°汾���ļ���ϣ�б�
	if (!ver.RequestHashList())
	{
		printf("Request Latest HashList failed.\n");
		return 1;
	}

	// �����ļ���ϣ��ͬ����ȡ���С����¡����ļ�
	if (!ver.DownloadLatestFiles(TMPDOWN_DIR))
	{
		printf("Download Latest Files failed.\n");
		return 1;
	}

	// ����ʱĿ¼�е��ļ��滻��װ��Ŀ¼���ļ�
	if (!ver.ReplaceFiles(TMPDOWN_DIR))
	{ 
		printf("Replace Files failed.\n");
		ver.RollBack();
		return 1;
	}
	
	return 0;
}
