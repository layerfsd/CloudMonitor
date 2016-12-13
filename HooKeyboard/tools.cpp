#include "tools.h"
#include <stdio.h>
#include <map>
#include <string>
#include <time.h>
#include <windows.h>

using namespace std;

struct TASK
{
	CHAR	path[MAX_PATH];
	size_t  ltime;
	DWORD	len;
	BOOL    status;
};
#define MAX_QUEUE_SIZE 1024

#pragma data_seg("GV_ALBERT_QUEUE")

size_t gll_head = 0;
size_t gll_tail = 0;
TASK gll_queue[MAX_QUEUE_SIZE] = {0};


#pragma data_seg()
#pragma comment(linker,"/SECTION:GV_ALBERT_QUEUE,RWS")


#define SEM_NAME	"ALBERT_SYNC"
#define SERV_ADDR  "127.0.0.1"
#define SERV_PORT	50006
#define SLEEP_TIME	500
#define MIN_SENT_INTERVAL 30  // 最短发送间隔时间(秒)

BOOL		KEEP_RUNNING = TRUE;
BOOL		isConnectionOK = FALSE;
SOCKET      GLOBAL_SOCKET = { 0 };


#pragma comment(lib,"ws2_32.lib")		// 建立socket()套接字

//BOOL SetCache(LPCSTR lpFilePath);
//BOOL GetCache(char* lpBuf, size_t bufSize);


// 通过此 TCP 接口通报消息
// 初始化 TCP 连接,后期需要加上双向身份认证功能
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
	//printf("Try Connect to %s:%d ...\n", SERV_ADDR, SERV_PORT);
	if (connect(GLOBAL_SOCKET, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) != 0)
	{
		//printf("Connect fail!\n");
		return FALSE;
	}
	else
	{
		isConnectionOK = TRUE;
	}
	printf("Connected\n");

	return TRUE;
}


// CreateFile过滤模块采用 TCP 本地通信模式
// 当通信另外一端未启动或者网络错误时,宿主程序会卡在 CreateFile() 系统调用中
// 因此在本地服务中创建一个·命名管道·以缓存需要通知的事件
// 其工作原理是: 接收到 Hook 通知后,将·路径·记录到队列中立刻返回
// 由一个专有线程负责从队列中读取数据通知另外一个程序
VOID TellBackend(const char* lPath, int length)
{
	static CHAR  lastPath[MAX_PATH] = { 0 };
	CHAR  tmpBuf[MAX_PATH];

	// 防止连续发送相同的文件名
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

static 	map<string,size_t> LastTask;

BOOL GetTask(TASK* tsk)
{
	static HANDLE semhd = OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, SEM_NAME);

	TASK cur, nxt;

	if (NULL == semhd)
	{
		printf("OpenSemaphore Error.");
		return FALSE;
	}

	BOOL bRet = FALSE;
	// acquire lock: gll_tail

	//printf("------------->>>WaitForSingleObject\n");
	WaitForSingleObject(semhd, INFINITE);
	if (gll_head == gll_tail)
	{
		// printf("<<<-------------ReleaseSemaphore\n");
		ReleaseSemaphore(semhd, 1, NULL);
		// release lock: gll_tail
		return bRet;
	}
	ReleaseSemaphore(semhd, 1, NULL);
	//printf("<<<-------------ReleaseSemaphore\n");

	DWORD nxtPos = 0;

	//printf("IN FOR\n");
	for ( ; gll_head != gll_tail; )
	{
		memset(&cur, 0, sizeof(TASK));
		memset(&nxt, 0, sizeof(TASK));

		nxtPos = (gll_head + 1) % MAX_QUEUE_SIZE;

		cur = gll_queue[gll_head];
		nxt = gll_queue[nxtPos];
		gll_head = nxtPos;

		//printf("head: %d next: %d tail: %d len:%d\n", gll_head, nxtPos, gll_tail, cur.len);
		// this == last
		if ( (cur.len == nxt.len) && !memcmp(cur.path, nxt.path, cur.len) )
		{
			continue;
		}
		else
		{
			bRet = TRUE;
			break;
		}
	}
	// printf("OUT FOR\n");
	
	if (bRet)	// 成功领取到任务
	{
		// 在一定时间内,不允许重复发送.
		// 检测是否与上一个任务一致
		if (cur.ltime - LastTask[cur.path] < MIN_SENT_INTERVAL)
		{
			bRet = FALSE;
		}
		else
		{
			memset(tsk, 0, sizeof(TASK));
			*tsk = cur;
			LastTask[cur.path] = cur.ltime;
			gll_head = nxtPos;
		}
		//MessageBox(NULL, lpFilePath, "Release SEM", MB_OK);
	}


	return bRet;
}

VOID AddTask(CONST CHAR* lpFilePath, DWORD dwSize)
{
	static HANDLE semhd = OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, SEM_NAME);

	if (NULL == semhd)
	{
		printf("OpenSemaphore Error.");
		return;
	}

	// 队列已满
	WaitForSingleObject(semhd, INFINITE);
	if (gll_head == (gll_tail + 1) % MAX_QUEUE_SIZE)
	{
		ReleaseSemaphore(semhd, 1, NULL);
		return;
	}
	ReleaseSemaphore(semhd, 1, NULL);

	// 字符串长度非法
	if (dwSize <= 0)
	{
		return;
	}

	TASK tTask;
	memset(&tTask, 0, sizeof(tTask));
	
	memcpy(tTask.path, lpFilePath, dwSize);
	tTask.ltime = (size_t)time(NULL);
	tTask.len = dwSize;
	tTask.status = FALSE;

	// acquire lock: gll_tail
	WaitForSingleObject(semhd, INFINITE);

	gll_queue[gll_tail] = tTask;
	gll_tail = (gll_tail + 1) % MAX_QUEUE_SIZE;
	
	ReleaseSemaphore(semhd, 1, NULL);
	//MessageBox(NULL, lpFilePath, "Tell Backend", MB_OK);

	// release lock: gll_tail
	return;
}


// 判断是否为 指定格式的文件
BOOL ProcessFilePath(LPCSTR lpFilePath)
{
	static LPCSTR matchList[] = {
		".rtf",

		".xls",
		".xlsx",

		".ppt",
		".pptx",

		".pdf",
		".doc",
		".docx",
		".wps",
	};

	//SetCache(lpFilePath);


	DWORD dwMatchListLen = sizeof(matchList) / sizeof(matchList[0]);

	if (NULL == lpFilePath)
	{
		return FALSE;
	}

	// 粗略过滤wps产生的临时路径
	if (NULL != strstr(lpFilePath, "\\\\?"))
	{
		return FALSE;
	}

	if (NULL != strstr(lpFilePath, "~$"))
	{
		return FALSE;
	}
	// 过滤完成

	//　获取文件后缀
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
			//TellBackend(lpFilePath, length);
			AddTask(lpFilePath, length);
			break;
		}
	}

	return retValue;
}


// 向后台程序发送一条信息
VOID SendMsg2Backend()
{
	static  TASK tsk = { 0 };
	CHAR	tmpBuf[MAX_PATH];
	DWORD  length;


	printf("isConnectionOK: %d\n", isConnectionOK);
	if (!InitTcpConnection())
	{
		printf("[ERROR] Not Connected to %s:%d\n", SERV_ADDR, SERV_PORT);
		isConnectionOK = FALSE;
	}


	HANDLE hd = CreateSemaphore(NULL, 1, 1, SEM_NAME);

	if (NULL == hd)
	{
		printf("Create [%s] failed.\n", SEM_NAME);
		return;
	}

	int sent = 0;
	while (KEEP_RUNNING)
	{
		Sleep(SLEEP_TIME);

		length = 0;

		if (GetTask(&tsk))
		{
			int   MaxRetryTime = 0;
			while ((tsk.status != TRUE) && (MaxRetryTime++ < MAX_RETRY_TIME))
			{
				//MessageBox(NULL, tPath, "Tell Backend", MB_OK);
				printf("[SEND:%d] %s\n", tsk.ltime, tsk.path);
				sent = send(GLOBAL_SOCKET, tsk.path, tsk.len, 0);

				if (sent <= 0)
				{
					// 如果发送失败，设置网络状况为:已断开连接
					if (sent < 0)  // TCP连接失效
					{
						printf("[SENT-FAILED:]\n");
						isConnectionOK = FALSE;
					}
				}
				else  // 发送成功
				{
					printf("[SENT-OK:]\n");
					length = recv(GLOBAL_SOCKET, tmpBuf, sizeof(tmpBuf), 0);
					tsk.status = true;		// 如果发送成功,标记发送状态为 真
				}

				if (!tsk.status)
				{
					// 尝试重新连接之前，先等待一会儿
					Sleep(MaxRetryTime * 1000);
					isConnectionOK = FALSE;
					InitTcpConnection();
				}

			}
		}

	}// end while
	
	if (!KEEP_RUNNING)
	{
		send(GLOBAL_SOCKET, "BYE", 3, 0);
	}

	if (NULL != hd)
	{
		printf("Closing Semaphore ...\n");
		printf("Thread-[bye-bye]\n");
		CloseHandle(hd);
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
