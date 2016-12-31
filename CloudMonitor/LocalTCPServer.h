#pragma once

#include <winsock2.h>
#include <windows.h>
#include <string>

using namespace std;


// 在本地开启一个端口，用来与‘IO监控’模块通信：
// 获取‘文件打开事件’，下发‘网络控制指令’
DWORD WINAPI ThreadProc(LPVOID lpParam);

// 远程控制--->关闭本地网络
bool RemoteShutdownNetwork(string& message, string& args);