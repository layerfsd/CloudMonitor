#ifndef _NAMED_PIPE_CLIENT_H
#define _NAMED_PIPE_CLIENT_H
#include <Windows.h>
#include <iostream>

using namespace std;
//用来保存在客户端通过 CreateFile 打开的命名管道句柄



// 创建命名管道
bool CreateNamedPipeInServer();

// 服务端从命名管道中写入数据
bool GetNamedPipeMessage(char* pReadBuf);


#endif
