#ifndef _NAMED_PIPE_CLIENT_H
#define _NAMED_PIPE_CLIENT_H
#include <Windows.h>
#include <iostream>

using namespace std;
//���������ڿͻ���ͨ�� CreateFile �򿪵������ܵ����



// ���������ܵ�
bool CreateNamedPipeInServer();

// ����˴������ܵ���д������
bool GetNamedPipeMessage(char* pReadBuf);


#endif
