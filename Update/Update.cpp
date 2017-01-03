// Update.cpp : 定义控制台应用程序的入口点。
//编译中遇到了“无法解析的外部符号”的错误，在以下链接中找到了解决方法--->在‘预处理’中加入两条宏
// http://www.cnblogs.com/ytjjyy/archive/2012/05/19/2508803.html

#include "stdafx.h"


static void SetWorkPath()
{
	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "\\..\\");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectory(strModule);
}

int main()
{
	// 设定程序工作目录
	SetWorkPath();
	//StopMyService();
	//bool bRet = GetFilesList("output.txt");
	

	CloudVersion ver;

	// 获取当前程序的版本号
	cout << ver.GetCurVersion() << endl;

	// 获取服务端保存的最新版本号
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
	// 更新本地版本号文件
	// ver.SetLatestVersion2File();
	
	return 0;
}
