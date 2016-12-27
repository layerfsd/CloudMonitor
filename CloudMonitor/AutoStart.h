#pragma once
#ifndef _AUTO_START__
#define _AUTO_START__

#if       _WIN32_WINNT < 0x0500
#undef  _WIN32_WINNT
#define _WIN32_WINNT   0x0500
#endif
#include <Windows.h>

#define STORE_AUTH_PATH		"SOFTWARE\\CloudMonitor"
#define AUTH_NAME			"Auth"
#define STARTUP_PATH		 "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define STARTUP_ARGS		"--autostart"


#define RUNNING_FLAG		"isStarted"
#define AUTHORIZED_FLAG		"isAuth"

typedef unsigned char uchar;


struct Account
{
	char username[32];
	char password[32];
};

// 将本程序加入开机启动项
// HKCU\Software\Microsoft\Windows\CurrentVersion\Run
BOOL RegisterProgram();

// 将认证用户名与密码加密后存储在注册表中
bool SetAuth(Account* act);

// 从注册表中 读取认证用户名与密码
bool GetAuth(Account* act);

BOOL GetRegFlag(LPCSTR regName, BOOL* data);

BOOL SetRegFlag(LPCSTR regName, BOOL data);

#endif