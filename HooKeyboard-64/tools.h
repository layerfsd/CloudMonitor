#pragma once

#ifndef  MY_TOOLS_INCLUDE
#define MY_TOOLS_INCLUDE

// 设置最大重复通知次数
#define MAX_RETRY_TIME	5

#include <Windows.h>


// 初始化 TCP 连接,后期需要加上双向身份认证功能
BOOL InitTcpConnection();


// 向后台程序发送一条信息
VOID SendMsg2Backend();


// 判断是否为一个"敏感文件"
BOOL ProcessFilePath(LPCSTR lpFilePath);


// 检查 tcp 连接目的IP 是否符合规范
BOOL CheckSockAddr(const struct sockaddr FAR *saddr);

// 接收来自本地进程的任务
// 非阻塞
void CheckTaskFromLocal(SOCKET sock);


bool FormatTime(char *buf, int bufSize);


// 调试输出函数
#define ALBERT_DEBUG
#include <stdio.h>
inline void XTrace(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#if defined(ALBERT_DEBUG) || defined(SHOW_DEBUG_MESSAGES)
	printf(fmt, args);
#endif
	va_end(args);
}

// 不定参数的 HOOK 日志记录
static int WriteToLog(char* fmt, ...);

#endif // ! MY_TOOLS_INCLUDE
