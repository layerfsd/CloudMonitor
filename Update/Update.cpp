// Update.cpp : �������̨Ӧ�ó������ڵ㡣
//�����������ˡ��޷��������ⲿ���š��Ĵ����������������ҵ��˽������--->�ڡ�Ԥ�����м���������
// http://www.cnblogs.com/ytjjyy/archive/2012/05/19/2508803.html

#include "stdafx.h"

int main()
{
	//StopMyService();
	bool bRet = GetFilesList("output.txt");
	
	printf("bRet: %d\n", bRet);

    return 0;
}
