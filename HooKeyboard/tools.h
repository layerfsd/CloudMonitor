#pragma once
#ifndef  MY_TOOLS_INCLUDE
#define MY_TOOLS_INCLUDE

// 设置最大重复通知次数
#define MAX_RETRY_TIME	15	

#include <Windows.h>


// 初始化 TCP 连接,后期需要加上双向身份认证功能
BOOL InitTcpConnection();


// 向后台程序发送一条信息
VOID SendMsg2Backend();


// 判断是否为一个"敏感文件"
BOOL ProcessFilePath(LPCSTR lpFilePath);

#endif // ! MY_TOOLS_INCLUDE
