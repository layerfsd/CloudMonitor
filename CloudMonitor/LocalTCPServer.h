#pragma once

#include <winsock2.h>
#include <windows.h>
#include <string>

using namespace std;


// �ڱ��ؿ���һ���˿ڣ������롮IO��ء�ģ��ͨ�ţ�
// ��ȡ���ļ����¼������·����������ָ�
DWORD WINAPI ThreadProc(LPVOID lpParam);

// Զ�̿���--->�رձ�������
bool RemoteShutdownNetwork(string& message, string& args);