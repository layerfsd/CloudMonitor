// Update.cpp : �������̨Ӧ�ó������ڵ㡣
//�����������ˡ��޷��������ⲿ���š��Ĵ����������������ҵ��˽������--->�ڡ�Ԥ�����м���������
// http://www.cnblogs.com/ytjjyy/archive/2012/05/19/2508803.html

#include "stdafx.h"


static void SetWorkPath()
{
	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //�õ���ǰģ��·��
	strcat(strModule, "\\..\\");     //����Ϊ��ǰ����·��Ϊ��ʱ����һ��
	SetCurrentDirectory(strModule);
}

int main()
{
	// �趨������Ŀ¼
	SetWorkPath();
	//StopMyService();
	//bool bRet = GetFilesList("output.txt");
	

	CloudVersion ver;

	// ��ȡ��ǰ����İ汾��
	cout << ver.GetCurVersion() << endl;

	// ��ȡ����˱�������°汾��
	ver.GetLatestVersion();

	if (!ver.WhetherUpdate())
	{
		printf("Already Latest Version.\n");
		return 0;
	}

	if (!ver.RequestHashList())
	{
		printf("Request Latest HashList failed.\n");
		return 1;
	}
	// ���±��ذ汾���ļ�
	// ver.SetLatestVersion2File();
	
	return 0;
}
