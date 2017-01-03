// Update.cpp : 定义控制台应用程序的入口点。
//编译中遇到了“无法解析的外部符号”的错误，在以下链接中找到了解决方法--->在‘预处理’中加入两条宏
// http://www.cnblogs.com/ytjjyy/archive/2012/05/19/2508803.html

#include "stdafx.h"


int main()
{
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

	// 获取最新版本的文件哈希列表
	if (!ver.RequestHashList())
	{
		printf("Request Latest HashList failed.\n");
		return 1;
	}

	// 根据文件哈希不同，获取所有‘更新’的文件
	ver.DownloadLatestFiles(TMPDOWN_DIR);

	// 用临时目录中的文件替换安装根目录的文件
	ver.ReplaceFiles(TMPDOWN_DIR);

	// 替换本地哈希文件
	//ReplaceFile();

	// 把最新版本号写入到本地文件
	// ver.SetLatestVersion2File();
	
	return 0;
}
