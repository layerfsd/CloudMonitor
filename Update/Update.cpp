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
	cout << ver.GetCurVersion() << endl;

	// ��ȡ����˱�������°汾��
	ver.GetLatestVersion();

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
	ver.DownloadLatestFiles(TMPDOWN_DIR);

	// ����ʱĿ¼�е��ļ��滻��װ��Ŀ¼���ļ�
	ver.ReplaceFiles(TMPDOWN_DIR);

	// �滻���ع�ϣ�ļ�
	//ReplaceFile();

	// �����°汾��д�뵽�����ļ�
	// ver.SetLatestVersion2File();
	
	return 0;
}
