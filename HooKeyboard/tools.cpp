#include "tools.h"
#include <stdio.h>
#include "NamePipe.h"


struct TASK
{
	CHAR	path[MAX_PATH];
	time_t  ltime;
	DWORD	len;
	BOOL    status;
};
#define MAX_QUEUE_SIZE 1024

#pragma data_seg("GV_ALBERT_QUEUE")

size_t gll_head = 0;
size_t gll_tail = 0;
TASK gll_queue[MAX_QUEUE_SIZE];
memset(&g_queue, 0, sizeof(gll_queue));

#pragma data_seg()
#pragma comment(linker,"/SECTION:GV_ALBERT_QUEUE,RWS")


#define SERV_ADDR  "127.0.0.1"
#define SERV_PORT	50006
#define SLEEP_TIME	3 * 1000

SOCKET    GLOBAL_SOCKET;
BOOL	  isConnectionOK = FALSE;



#pragma comment(lib,"ws2_32.lib")		// ����socket()�׽���


//BOOL SetCache(LPCSTR lpFilePath);
//BOOL GetCache(char* lpBuf, size_t bufSize);


// ͨ���� TCP �ӿ�ͨ����Ϣ
// ��ʼ�� TCP ����,������Ҫ����˫�������֤����
BOOL InitTcpConnection()
{
	if (isConnectionOK)
	{
		//printf("[OK] Connected to %s:%d\n", SERV_ADDR, SERV_PORT);
		return TRUE;
	}

	SOCKADDR_IN serveraddr;

	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);	//initial Ws2_32.dll by a process

	if ((GLOBAL_SOCKET = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)	//create a tcp socket
	{
		printf("Create socket fail!\n");
		return FALSE;
	}

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERV_PORT);
	serveraddr.sin_addr.S_un.S_addr = inet_addr(SERV_ADDR);

	//connect to server
	printf("Try Connect to %s:%d ...\n", SERV_ADDR, SERV_PORT);
	if (connect(GLOBAL_SOCKET, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) != 0)
	{
		printf("Connect fail!\n");
		return FALSE;
	}
	else
	{
		isConnectionOK = TRUE;
	}
	printf("Connected\n");

	return TRUE;
}


// CreateFile����ģ����� TCP ����ͨ��ģʽ
// ��ͨ������һ��δ���������������ʱ,��������Ῠ�� CreateFile() ϵͳ������
// ����ڱ��ط����д���һ���������ܵ����Ի�����Ҫ֪ͨ���¼�
// �乤��ԭ����: ���յ� Hook ֪ͨ��,����·������¼�����������̷���
// ��һ��ר���̸߳���Ӷ����ж�ȡ����֪ͨ����һ������
VOID TellBackend(const char* lPath, int length)
{
	static CHAR  lastPath[MAX_PATH] = { 0 };
	CHAR  tmpBuf[MAX_PATH];

	// ��ֹ����������ͬ���ļ���
	if (0 == strcmp(lPath, lastPath))
	{
		return;
	}
	else
	{
		memcpy(lastPath, lPath, length);
	}

	if (!InitTcpConnection())
	{
		isConnectionOK = FALSE;
		return;
	}
	//MessageBox(NULL, lPath, "Tell Backend", MB_OK);
	//MessageBox(NULL, lastPath, "lastPath", MB_OK);
	if (send(GLOBAL_SOCKET, lPath, length, 0) <= 0)
	{
		return;
	}
	else
	{
		recv(GLOBAL_SOCKET, tmpBuf, sizeof(tmpBuf), 0);
		//MessageBox(NULL, lPath, "Tell Backend", MB_OK);
	}
	return;
}

BOOL GetTask(LPCSTR lpFilePath, DWORD* dwSize)
{
	if (gll_head == (gll_tail + 1) % MAX_QUEUE_SIZE)
	{
		return
	}

	*dwSize = gll_queue[gll_head].len;
	memcpy(lpFilePath, gll_queue[gll_head].path, gll_queue[gll_head].len);
	gll_head = (gll_head + 1) % MAX_QUEUE_SIZE;

	return;
}

VOID AddTask(LPCSTR lpFilePath, DWORD dwSize)
{
	// ��������
	if (gll_head == (gll_tail + 1) % MAX_QUEUE_SIZE)
	{
		return;
	}
	// �ַ������ȷǷ�
	if (dwSize <= 0)
	{
		return;
	}

	TASK tTask;
	memset(&tTask, 0, sizeof(tTask));
	
	memcpy(tTask.path, lpFilePath, dwSize);
	tTask.ltime = time(NULL);
	tTask.len = dwSize;
	tTask.status = TRUE;

	// acquire lock: gll_tail
	gll_queue[gll_tail] = tTask;
	gll_tail = (gll_tail + 1) % MAX_QUEUE_SIZE;
	// release lock: gll_tail
	return;
}


// �ж��Ƿ�Ϊһ��"�����ļ�"
BOOL ProcessFilePath(LPCSTR lpFilePath)
{
	static LPCSTR matchList[] = {
		".doc",
		".docx"
	};

	//SetCache(lpFilePath);


	DWORD dwMatchListLen = sizeof(matchList) / sizeof(matchList[0]);

	if (NULL == lpFilePath)
	{
		return FALSE;
	}

	// ���Թ���wps��������ʱ·��
	if (NULL != strstr(lpFilePath, "\\\\?"))
	{
		return FALSE;
	}

	if (NULL != strstr(lpFilePath, "~$"))
	{
		return FALSE;
	}
	// �������

	//����ȡ�ļ���׺
	LPCSTR pos = strrchr(lpFilePath, '.');

	if (NULL == pos)
	{
		return FALSE;
	}

	BOOL retValue = FALSE;

	for (DWORD i = 0; i < dwMatchListLen; i++)
	{
		if (!strcmp(pos, matchList[i]))
		{
			retValue = TRUE;
			//SetCache(lpFilePath);
			int length = strnlen(lpFilePath, MAX_PATH);
			//MessageBox(NULL, lpFilePath, "Tell Backend", MB_OK);
			TellBackend(lpFilePath, length);
			break;
		}
	}

	return retValue;
}



#if 0
BOOL SetCache(LPCSTR lpFilePath)
{
	//g_PathList.push(lpFilePath);
	//char text[1024];
	//sprintf(text, "%d %s", g_PathList.size(), lpFilePath);
	//MessageBox(NULL, text, "RUNNING-APP���·��", MB_OK);
	return TRUE;
}

BOOL GetCache(char* lpBuf, size_t bufSize)
{
	if (g_PathList.size() > 0)
	{
		strncpy(lpBuf, g_PathList.front().c_str(), bufSize);
		g_PathList.pop();

		return TRUE;
	}
	return FALSE;
}

// ���̨������һ����Ϣ
VOID SendMsg2Backend()
{
	char tPath[MAX_PATH] = "HELLO";
	CHAR  tmpBuf[MAX_PATH];
	int length;


	while (1)
	{
	_RESTART:
		Sleep(SLEEP_TIME);

		if (!InitTcpConnection())
		{
			printf("[ERROR] Not Connected to %s:%d\n", SERV_ADDR, SERV_PORT);
			isConnectionOK = FALSE;
			goto _RESTART;
		}

		if (GetCache(tPath, MAX_PATH))
		{
			length = strnlen(tPath, MAX_PATH);
			MessageBox(NULL, tPath, "Tell Backend", MB_OK);
			if (send(GLOBAL_SOCKET, tPath, length, 0) <= 0)
			{
				isConnectionOK = FALSE;
				goto _RESTART;
			}
			else
			{
				recv(GLOBAL_SOCKET, tmpBuf, sizeof(tmpBuf), 0);
			}
		}
	}
	return;
}


DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	//cout << "sub thread started\n" << endl;
	//CreateNamedPipeInServer();
	SendMsg2Backend();
	return 0;
}


#endif