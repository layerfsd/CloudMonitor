//Refer: http://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancediomethod5a.html

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#include <queue>

#include "network.h"
#include "LocalTCPServer.h"

#define DATA_BUFSIZE 1024

// main.cpp 
// 控制线程结束
extern BOOL g_RUNNING;		

// network.cpp
// 程序配置信息
extern AppConfig GS_acfg;

//  创建一个队列,用来缓存 路径列表
static queue<string> LocalPathList;

// 是否要关闭对外通讯
static BOOL isShutdownNetwork = FALSE;

static const int LocalControlNumLen = 3;
static map<string, string> LOCAL_CONTROL{
	{ "CMD_GOT", "100" },
	{ "STOP_SERVICE", "101" },
	{ "OPEN_NETWORK", "102" },
	{ "SHUT_NETWORK", "103" },

};

typedef struct _SOCKET_INFORMATION {
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	SOCKET Socket;
	OVERLAPPED Overlapped;
	DWORD BytesSEND;
	DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

// Prototypes
BOOL CreateSocketInformation(SOCKET s);
void FreeSocketInformation(DWORD Index);

// Global var
DWORD TotalSockets = 0;
LPSOCKET_INFORMATION SocketArray[4];

VOID InformAll(LPCSTR Buf, DWORD Length)
{
	for (DWORD i = 0; i < TotalSockets; i++)
	{
		LPSOCKET_INFORMATION SocketInfo = SocketArray[i];

		printf("[SEND:%d] %s to %d\n", Length, Buf, i);
		if (send(SocketInfo->Socket, Buf, Length, 0) == SOCKET_ERROR)
		{
			FreeSocketInformation(i);
		}
	}
	// end func
}


inline VOID SendCMD(string iter)
{
	InformAll(iter.c_str(), LocalControlNumLen);
}

// 远程控制--->向‘IO过滤中心’发送‘控制指令’
void ReverseControl()
{
	static BOOL changed = FALSE;
	if (changed != isShutdownNetwork)
	{
		printf("changed %d isShutdownNetwork %d\n", changed, isShutdownNetwork);
		changed = isShutdownNetwork;

		// 确定要关闭网络时，保持与服务端IP的正常通信
		if (isShutdownNetwork)
		{
			printf("Except for [%s]", GS_acfg.ServAddr);

			// 发送‘关闭网络’指令
			SendCMD(LOCAL_CONTROL["SHUT_NETWORK"]);
			// 发送需要额外处理的公网IP
			InformAll(GS_acfg.ServAddr, strnlen(GS_acfg.ServAddr, 32));
		}
		else
		{
			// 发送‘开启用户网络’指令
			printf("sending command %s\n", "OPEN_NETWORK");
			SendCMD(LOCAL_CONTROL["OPEN_NETWORK"]);
		}
	}
	return;
}

int LocalTCPServer()
{
	SOCKET ListenSocket;
	SOCKET AcceptSocket;
	SOCKADDR_IN InternetAddr;
	WSADATA wsaData;
	INT Ret;
	FD_SET WriteSet;
	FD_SET ReadSet;
	DWORD i;
	DWORD Total;
	ULONG NonBlock;
	DWORD Flags;
	DWORD SendBytes;
	DWORD RecvBytes;

	CHAR  CurPath[MAX_PATH];	// 临时处理‘本地路径’

	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup() failed with error %d\n", Ret);
		WSACleanup();
		return 1;
	}

	// Prepare a socket to listen for connections
	if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	InternetAddr.sin_family = AF_INET;

	inet_pton(AF_INET, "127.0.0.1", &InternetAddr.sin_addr);
	InternetAddr.sin_port = htons(LOCAL_TCP_PORT);

	if (bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	if (listen(ListenSocket, 5))
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	// Change the socket mode on the listening socket from blocking to
	// non-block so the application will not block waiting for requests
	NonBlock = 1;
	if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	while (TRUE)
	{
		// 通知IO过滤中心，停止服务
		if (!g_RUNNING)
		{
			printf("tell IO Center to Stop [%s]\n", "STOP_SERVICE");
			SendCMD(LOCAL_CONTROL["STOP_SERVICE"]);
			Sleep(10 * 1000);
			break;
		}
		
		// 处理‘反向控制’
		ReverseControl();

		// Prepare the Read and Write socket sets for network I/O notification
		FD_ZERO(&ReadSet);
		FD_ZERO(&WriteSet);

		// Always look for connection attempts
		FD_SET(ListenSocket, &ReadSet);

		// Set Read and Write notification for each socket based on the
		// current state the buffer.  If there is data remaining in the
		// buffer then set the Write set otherwise the Read set
		for (i = 0; i < TotalSockets; i++)
		{
			if (SocketArray[i]->BytesRECV > SocketArray[i]->BytesSEND)
				FD_SET(SocketArray[i]->Socket, &WriteSet);
			else
				FD_SET(SocketArray[i]->Socket, &ReadSet);
		}

		if ((Total = select(0, &ReadSet, &WriteSet, NULL, NULL)) == SOCKET_ERROR)
		{
			printf("select() returned with error %d\n", WSAGetLastError());
			return 1;
		}

		// Check for arriving connections on the listening socket.
		if (FD_ISSET(ListenSocket, &ReadSet))
		{
			Total--;
			if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) != INVALID_SOCKET)
			{
				// Set the accepted socket to non-blocking mode so the server will
				// not get caught in a blocked condition on WSASends
				NonBlock = 1;
				if (ioctlsocket(AcceptSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
				{
					printf("ioctlsocket(FIONBIO) failed with error %d\n", WSAGetLastError());
					return 1;
				}

				if (CreateSocketInformation(AcceptSocket) == FALSE)
				{
					printf("CreateSocketInformation(AcceptSocket) failed!\n");
					return 1;
				}

			}
			else
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					printf("accept() failed with error %d\n", WSAGetLastError());
					return 1;
				}
			}
		}

		// Check each socket for Read and Write notification until the number
		// of sockets in Total is satisfied
		for (i = 0; Total > 0 && i < TotalSockets; i++)
		{
			LPSOCKET_INFORMATION SocketInfo = SocketArray[i];

			// If the ReadSet is marked for this socket then this means data
			// is available to be read on the socket
			if (FD_ISSET(SocketInfo->Socket, &ReadSet))
			{
				Total--;

				SocketInfo->DataBuf.buf = SocketInfo->Buffer;
				SocketInfo->DataBuf.len = DATA_BUFSIZE;

				Flags = 0;
				if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("WSARecv() failed with error %d\n", WSAGetLastError());
						FreeSocketInformation(i);
					}
					continue;
				}
				else
				{
					SocketInfo->BytesRECV = RecvBytes;

					// If zero bytes are received, this indicates the peer closed the connection.
					if (RecvBytes == 0)
					{
						FreeSocketInformation(i);
						continue;
					}
				}
			}

			// If the WriteSet is marked on this socket then this means the internal
			// data buffers are available for more data
			if (FD_ISSET(SocketInfo->Socket, &WriteSet))
			{
				Total--;

				SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
				SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;

				// 忽略‘心跳包’）
				if (0 == strncmp(SocketInfo->Buffer, "HBT", 3))
				{
					SocketInfo->BytesSEND = 0;
					SocketInfo->BytesRECV = 0;
					continue;
				}
				//保存‘IO事件详情’
				else 
				{
					memset(CurPath, 0, sizeof(CurPath));
					memcpy(CurPath, SocketInfo->DataBuf.buf, SocketInfo->DataBuf.len);
					printf("[HOOK-GET] %s [LENGTH:%d]\n", CurPath, SocketInfo->DataBuf.len);
					LocalPathList.push(CurPath);
				}

				if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("WSASend() failed with error %d\n", WSAGetLastError());
						FreeSocketInformation(i);
					}
					continue;
				}
				else
				{
					SocketInfo->BytesSEND += SendBytes;

					if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
					{
						SocketInfo->BytesSEND = 0;
						SocketInfo->BytesRECV = 0;
					}
				}
			}
		}
	}
	return 0;
}

BOOL CreateSocketInformation(SOCKET s)
{
	LPSOCKET_INFORMATION SI;

	printf("Accepted socket number %d\n", s);

	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return FALSE;
	}
	else
		printf("GlobalAlloc() for SOCKET_INFORMATION is OK!\n");

	// Prepare SocketInfo structure for use
	SI->Socket = s;
	SI->BytesSEND = 0;
	SI->BytesRECV = 0;

	SocketArray[TotalSockets] = SI;
	TotalSockets++;
	return(TRUE);
}

void FreeSocketInformation(DWORD Index)
{
	LPSOCKET_INFORMATION SI = SocketArray[Index];
	DWORD i;

	closesocket(SI->Socket);
	printf("Closing socket number %d\n", SI->Socket);
	GlobalFree(SI);

	// Squash the socket array
	for (i = Index; i < TotalSockets; i++)
	{
		SocketArray[i] = SocketArray[i + 1];
	}

	TotalSockets--;
}


DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	LocalTCPServer();
	return 0;
}


bool GetInformMessage(char *buf, size_t bufSize)
{
	if (LocalPathList.size() > 0)
	{
		strncpy(buf, LocalPathList.front().c_str(), bufSize);
		LocalPathList.pop();
		return true;
	}

	return false;
}

// 关闭本地网络
bool RemoteShutdownNetwork(string& message, string& args)
{
	if ("SHUT" == args)
	{
		isShutdownNetwork = TRUE;
	}
	else if ("OPEN" == args)
	{
		isShutdownNetwork = FALSE;
	}
	printf("isShutdownNetwork %d\n", isShutdownNetwork);
	return true;
}
