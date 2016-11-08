// testSelect.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <WinSock2.h>
#include <iostream>
using namespace std;

#include <stdio.h>

#pragma comment(lib,"ws2_32.lib")

#define PORT		8000
#define MSGSIZE		255
#define SRV_IP		"0.0.0.0"

//#define ORIGINAL 1

#if ORIGINAL
int g_nSockConn = 0;//�������ӵ���Ŀ

//FD_SETSIZE����winsocket2.hͷ�ļ��ﶨ��ģ�����windowsĬ�����Ϊ64
//�ڰ���winsocket2.hͷ�ļ�ǰʹ�ú궨������޸����ֵ


struct ClientInfo
{
	SOCKET sockClient;
	SOCKADDR_IN addrClient;
};

ClientInfo g_Client[FD_SETSIZE];

DWORD WINAPI WorkThread(LPVOID lpParameter);

int _tmain(int argc, _TCHAR* argv[])
{//��������Ͳ������ˣ������̻�����ƪ�����ｲ�ĺ���ϸ��
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr(SRV_IP);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(PORT);

	bind(sockListen, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));

	listen(sockListen, 64);

	DWORD dwThreadIDRecv = 0;
	DWORD dwThreadIDWrite = 0;

	HANDLE hand = CreateThread(NULL, 0, WorkThread, NULL, 0, &dwThreadIDRecv);//���������շ���Ϣ�Ľ���
	if (hand == NULL)
	{
		cout << "Create work thread failed\n";
		getchar();
		return -1;
	}

	SOCKET sockClient;
	SOCKADDR_IN addrClient;
	int nLenAddrClient = sizeof(SOCKADDR);//������0���Ի����˰�����ҳ�����

	while (true)
	{
		sockClient = accept(sockListen, (SOCKADDR*)&addrClient, &nLenAddrClient);//����������һ��Ҫ����addrClient��С��ʼ��
		//��������ߵĵ�ַ��Ϣ
		cout<<inet_ntoa(addrClient.sin_addr)<<":"<<ntohs(addrClient.sin_port)<<"has connect !"<<endl;

		if (sockClient != INVALID_SOCKET)
		{
			g_Client[g_nSockConn].addrClient = addrClient;//�������Ӷ˵�ַ��Ϣ
			g_Client[g_nSockConn].sockClient = sockClient;//���������߶���
			g_nSockConn++;
		}


	}

	closesocket(sockListen);
	WSACleanup();

	return 0;
}


DWORD WINAPI WorkThread(LPVOID lpParameter)
{
	FD_SET fdRead;
	int nRet = 0;//��¼���ͻ��߽��ܵ��ֽ���
	TIMEVAL tv;//���ó�ʱ�ȴ�ʱ��
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	char buf[MSGSIZE] = "";

	while (true)
	{
		FD_ZERO(&fdRead);
		for (int i = 0; i < g_nSockConn; i++)
		{
			FD_SET(g_Client[i].sockClient, &fdRead);
		}

		//ֻ����read�¼����������滹�ǻ��ж�д��Ϣ���͵�
		nRet = select(0, &fdRead, NULL, NULL, &tv);

		if (nRet == 0)
		{//û�����ӻ���û�ж��¼�
			continue;
		}

		for (int i = 0; i < g_nSockConn; i++)
		{
			if (FD_ISSET(g_Client[i].sockClient, &fdRead))
			{
				memset(buf, 0, sizeof(buf));
				nRet = recv(g_Client[i].sockClient, buf, sizeof(buf), 0);

				if (nRet == 0 || (nRet == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET))
				{
					cout << "Client " << inet_ntoa(g_Client[i].addrClient.sin_addr) << "closed" << endl;
					closesocket(g_Client[i].sockClient);

					//��ʧЧ��sockClient�޳�������������һ������ȥ
					//g_Client[i--].sockClient = g_Client[--g_nSockConn].sockClient;
					g_Client[i].sockClient = 0;
					g_nSockConn -= 1;
				}
				else
				{
					cout << inet_ntoa(g_Client[i].addrClient.sin_addr) << ": " << endl;
					cout << buf << endl;
					cout << "Server:" << endl;
					//gets(buf);
					//strcpy(buf, "Hello!");
					nRet = send(g_Client[i].sockClient, buf, strlen(buf) + 1, 0);
				}
			}//end for-if
		} //end for
	}
	return 0;
}
#else


bool GetServerInfo(SOCKET& sockfd, char *buf, size_t bufSize)
{
	static	FD_SET fdRead;
	int		nRet = 0;//��¼���ͻ��߽��ܵ��ֽ���
	static TIMEVAL	tv = { 0, 500 };//���ó�ʱ�ȴ�ʱ��

	FD_ZERO(&fdRead);

	FD_SET(sockfd, &fdRead);

	//ֻ����read�¼����������滹�ǻ��ж�д��Ϣ���͵�
	nRet = select(0, &fdRead, NULL, NULL, &tv);

	if (nRet == 0)
	{//û�����ӻ���û�ж��¼�
		return false;
	}

	memset(buf, 0, bufSize);
	nRet = recv(sockfd, buf, bufSize, 0);
	//nRet = send(sockfd, buf, nRet, 0);

	if (nRet > 0)
	{
		return true;
	}

	cout << "Disconnected from Server! " << endl;
	closesocket(sockfd);
	return false;
}


int _tmain(int argc, _TCHAR* argv[])
{//��������Ͳ������ˣ������̻�����ƪ�����ｲ�ĺ���ϸ��
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr(SRV_IP);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(PORT);

	bind(sockListen, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));

	listen(sockListen, 2);

	DWORD dwThreadIDRecv = 0;
	DWORD dwThreadIDWrite = 0;

	SOCKET sockClient;
	SOCKADDR_IN addrClient;
	int nLenAddrClient = sizeof(SOCKADDR);//������0���Ի����˰�����ҳ�����

	cout << "Waiting for client ..." << endl;
	sockClient = accept(sockListen, (SOCKADDR*)&addrClient, &nLenAddrClient);//����������һ��Ҫ����addrClient��С��ʼ��
	cout << inet_ntoa(addrClient.sin_addr) << ":" << ntohs(addrClient.sin_port) << "has connect !" << endl;

	char buf[256];
	while (true)
	{																		 
		if (GetServerInfo(sockClient, buf, sizeof(buf)))
		{
			cout << "recv: " << buf << endl;
		}
		cout << "deal with others ..." << endl;
		Sleep(500);
	}

	closesocket(sockListen);
	WSACleanup();

	return 0;
}
#endif
