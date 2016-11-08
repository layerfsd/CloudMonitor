#include "stdafx.h"

#pragma comment(lib, "ws2_32.lib") 

void InitUDP();
int SendMsg(char* sendData);
void EndUDP();


SOCKET g_Client;
sockaddr_in g_ServInfo;

void InitUDP()
{
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;

	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		return ;
	}
	SOCKET g_Client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	g_ServInfo.sin_family = AF_INET;
	g_ServInfo.sin_port = htons(50006);
	g_ServInfo.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	return;
}


int SendMsg(char* sendData)
{
	printf("sending %s\n", sendData);
	sendto(g_Client, sendData, strlen(sendData), 0, (sockaddr *)&g_ServInfo, sizeof(sockaddr_in));
	//char recvData[255];
	//int ret = recvfrom(sclient, recvData, 255, 0, (sockaddr *)&sin, &len);
	//if (ret > 0)
	//{
	//	recvData[ret] = 0x00;
	//	printf(recvData);
	//}

	return 0;
}

void EndUDP()
{
	closesocket(g_Client);
	WSACleanup();
	return ;
}