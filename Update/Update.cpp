// Update.cpp : 定义控制台应用程序的入口点。
//编译中遇到了“无法解析的外部符号”的错误，在以下链接中找到了解决方法--->在‘预处理’中加入两条宏
// http://www.cnblogs.com/ytjjyy/archive/2012/05/19/2508803.html

#include "stdafx.h"

int main()
{
	//StopMyService();
	bool bRet = GetFilesList("output.txt");
	
	printf("bRet: %d\n", bRet);

    return 0;
}
