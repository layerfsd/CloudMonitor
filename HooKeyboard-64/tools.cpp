#include "tools.h"
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <time.h>

#include <sys/types.h>  
#include <sys/stat.h>  

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

static BOOL isShutdownNetwork = FALSE;

#pragma data_seg()
#pragma comment(linker,"/SECTION:GV_ALBERT_QUEUE,RWS")


#define SEM_NAME	"ALBERT_SYNC"
#define SERV_ADDR   "127.0.0.1"
#define SERV_PORT	50006
#define ONE_SECOND	1000
#define MIN_SENT_INTERVAL 3  // 最短发送间隔时间

BOOL		KEEP_RUNNING = TRUE;
BOOL		isConnectionOK = FALSE;
SOCKET      GLOBAL_SOCKET = { 0 };

#define TRASH_FILE   "$RECYCLE.BIN"

static CHAR SYS_TMP_PATH[MAX_PATH];
static BOOL TMPPATH_GOT = false;

#pragma comment(lib,"ws2_32.lib")		// 建立socket()套接字


#define MIN_CREATETIME 5

static bool NotIgnoreByTime(const char* filename)
{
	struct	_stat buf;
	int		result;
	size_t	Curtime = 0;

	// Get data associated with "crt_stat.c":   
	result = _stat(filename, &buf);

	// Check if statistics are valid:   
	if (result != 0)
	{
		return false;
	}

	// get current time
	Curtime = (size_t)time(NULL);

	// 当前时间 - 创建时间 > MIN_CREATETIME
	if (Curtime - (size_t)buf.st_ctime > MIN_CREATETIME)
	{
		return true;
	}
	// 该文件为新创建的空白文件,不处理
	return false;
}



// 通过此 TCP 接口通报消息
// 初始化 TCP 连接,后期需要加上双向身份认证功能
BOOL InitTcpConnection()
{
	if (isConnectionOK)
	{
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

	ULONG NonBlock;

	NonBlock = 1;
	if (ioctlsocket(GLOBAL_SOCKET, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		return FALSE;
	}

	//connect to server
	int ret = connect(GLOBAL_SOCKET, (SOCKADDR *)&serveraddr, sizeof(serveraddr));

	if (ret != 0)
	{
		ret = FALSE;
	}
	else
	{
		ret = TRUE;
	}

	return ret;
}


// CreateFile过滤模块采用 TCP 本地通信模式
// 当通信另外一端未启动或者网络错误时,宿主程序会卡在 CreateFile() 系统调用中
// 因此在本地服务中创建一个·命名管道·以缓存需要通知的事件
// 其工作原理是: 接收到 Hook 通知后,将·路径·记录到队列中立刻返回
// 由一个专有线程负责从队列中读取数据通知另外一个程序

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


		// 如果当前路径与下一个路径相等，则跳过当前路径
		if (0  == memcmp(cur.path, nxt.path, cur.len) )
		{
			continue;
		}
		else
		{
			bRet = TRUE;
			break;
		}
	}
	
	if (bRet)	// 成功领取到任务
	{
		// 在一定时间内,不允许重复发送.
		// 检测是否与上一个任务一致

		// 检查该路径是否在 MIN_SENT_INTERVAL 秒内重复发送过
		for (auto it = LastTask.begin(); it != LastTask.end(); ++it)
		{
			if ((!memcmp(cur.path, it->first.c_str(), cur.len)) && (cur.ltime - it->second < MIN_SENT_INTERVAL))
			{
				bRet = FALSE;
				//printf("[REPEAT-IGNORED:] %s\n", it->first.c_str());
				break;
			}
		}

		if (bRet)
		{
			memset(tsk, 0, sizeof(TASK));
			*tsk = cur;
			LastTask[cur.path] = cur.ltime;
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



static unsigned char	servAddr[4] = {
	121, 42, 146, 43
};


// 检查 tcp 连接目的IP 是否符合规范
BOOL CheckSockAddr(const struct sockaddr FAR *saddr)
{
	// 如果不关闭用户网络，则直接返回TRUE
	if (!isShutdownNetwork)
	{
		return TRUE;
	}

	const SOCKADDR_IN *s = (SOCKADDR_IN *)saddr;
	unsigned char    data[4];
	BOOL    bRet = FALSE;

	memcpy(data, (unsigned char *)&(s->sin_addr), 4);

	if (0 == data[0])
	{
		bRet = TRUE;
	}
	else if (127 == data[0])
	{
		bRet = TRUE;
	}
	else if (10 == data[0])
	{
		bRet = TRUE;
	}
	else if (192 == data[0] && 168 == data[1])
	{
		bRet = TRUE;
	}
	else if (172 == data[0])
	{
		if (data[1] >= 16 && data[1] <= 31)
		{
			bRet = TRUE;
		}
	}
	else if (!memcmp(data, servAddr, 4))
	{
		bRet = TRUE;
	}
	else
	{
		bRet = FALSE;
	}
	// 如果开启了‘网络控制’，当有进程要访问外网时，弹出对话框
	//if (FALSE == bRet)
	//{
	//	char *strAddr;
	//	strAddr = inet_ntoa(s->sin_addr);
	//	char strModule[MAX_PATH];
	//	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径

	//	MessageBox(NULL, strAddr, strModule, MB_OK);
	//}

	return bRet;
}



// 判断是否为 指定格式的文件
BOOL ProcessFilePath(LPCSTR lpFilePath)
{
	static LPCSTR matchList[] = {
		//".txt",

		//".rtf",

		".xls",
		".xlsx",

		".ppt",
		".pptx",

		".pdf",
		".doc",
		".docx",
		".wps",
	};


	DWORD dwMatchListLen = sizeof(matchList) / sizeof(matchList[0]);

	if (NULL == lpFilePath)
	{
		return FALSE;
	}
	//　获取文件后缀
	LPCSTR pos = strrchr(lpFilePath, '.');

	if (NULL == pos || '\\' == lpFilePath[0])
	{
		return FALSE;
	}

	if (NULL != strchr(lpFilePath, '$'))
	{
		return FALSE;
	}

	//// 过滤系统临时文件
	//if (TMPPATH_GOT && !strstr(lpFilePath, SYS_TMP_PATH))
	//{
	//	return FALSE;
	//}



	BOOL retValue = FALSE;

	for (DWORD i = 0; i < dwMatchListLen; i++)
	{
		if (!strcmp(pos, matchList[i]))
		{
			retValue = TRUE;
			//SetCache(lpFilePath);
			int length = (int)strnlen(lpFilePath, MAX_PATH);
			//MessageBox(NULL, lpFilePath, "Tell Backend", MB_OK);
			//TellBackend(lpFilePath, length);
			AddTask(lpFilePath, length);
			break;
		}
	}

	return retValue;
}

// 与本地进程 CloudMonitor.exe 协商执行远程的控制指令
// 从 100 开始，可以有900 （1000-100=900）个控制指令
// 若从默认‘0’开始智能有 10（0-9）个控制指令，不方便扩展
enum {
	CMD_GOT = 100,	// 确认信息
	STOP_SERVICE,
	OPEN_NETWORK,
	SHUT_NETWORK
}LOCAL_CONTROL;

static const int LocalControlNumLen = 3;

// 接收来自 CloudMonitor.exe 的指令
void CheckTaskFromLocal(SOCKET sock)
{
	static	FD_SET fdRead;
	static  char   tmpBuf[128];
	static  char reply[] = "Yes Commander.";

	int		nRet = 0;		//记录发送或者接受的字节数
	static int	CMD = 0;


	memset(tmpBuf, 0, sizeof(tmpBuf));
	nRet = recv(GLOBAL_SOCKET, tmpBuf, LocalControlNumLen, 0);
	DWORD	netAddr;

	if (nRet > 0)
	{
		CMD = 0;

		// 把字符串形式的数字转换为数字
		tmpBuf[LocalControlNumLen] = 0;
		CMD = atoi(tmpBuf);
	
		if (STOP_SERVICE == CMD)			// 关闭当前程序
		{
			printf("[CMD] STOP SERVICE\n");
			KEEP_RUNNING = FALSE;
		}
		else if (OPEN_NETWORK == CMD)		// 开启客户端与 Internet 的网络通道
		{
			printf("[OPEN NETWORK]\n");
			isShutdownNetwork = FALSE;
		}
		else if (SHUT_NETWORK == CMD)	// 关闭客户端与 Internet 的网络通道,保留与 ‘远程服务器’的连接
		{
			isShutdownNetwork = TRUE;

			memset(tmpBuf, 0, sizeof(tmpBuf));
			nRet = recv(GLOBAL_SOCKET, tmpBuf, 32, 0);

			netAddr = inet_addr(tmpBuf);
			memcpy(servAddr, (char *)&netAddr, 4);

			printf("[SHUTDOWN NETWORK] EXCEPT FOR: [%s]\n", tmpBuf);
		}
	}

	return;
}



// 向后台程序发送一条信息
VOID SendMsg2Backend()
{
	static  TASK tsk = { 0 };
	CHAR	tmpBuf[MAX_PATH];
	DWORD  length;


	if (!InitTcpConnection())
	{
		isConnectionOK = FALSE;
	}


	HANDLE hd = CreateSemaphore(NULL, 1, 1, SEM_NAME);

	if (NULL == hd)
	{
		printf("Create [%s] failed.\n", SEM_NAME);
		return;
	}

	int sent = 0;
	int loopCount = 0;
	int reConnectTime = 0;
	char timeBuf[64];

	while (KEEP_RUNNING)
	{
		Sleep(ONE_SECOND);

		loopCount += 1;

		length = 0;
		CheckTaskFromLocal(GLOBAL_SOCKET);

		if (loopCount >= 3)	// 此处是一个本地心跳，保证与CloudMonitor的正常通信。间隔‘3’秒发送一次
		{
			if (reConnectTime >= MAX_RETRY_TIME)
			{
				KEEP_RUNNING = FALSE;
				break;
			}
			loopCount = 0;
			sent = send(GLOBAL_SOCKET, "HBT", 3, 0);

			if (sent <= 0)
			{
				printf("Connect to Local Failed\n");
				isConnectionOK = FALSE;
				reConnectTime += 1;
				printf("ReConnect [%d] time\n", reConnectTime);
				printf("Trying connect again...\n");
				InitTcpConnection();
			}
			else
			{
				reConnectTime = 0;
			}
			// 发送本地‘心跳包’后，无论成功与否，都跳过下面的代码
			continue;
		}
		
REGET_TASK:
		if (GetTask(&tsk))
		{
			if (!NotIgnoreByTime(tsk.path))
			{
				//printf("[IGNORED] %s\n", tsk.path);
				goto REGET_TASK;
			}


			int   MaxRetryTime = 0;
			while (KEEP_RUNNING && (tsk.status != TRUE) && (MaxRetryTime++ < MAX_RETRY_TIME))
			{
				//MessageBox(NULL, tPath, "Tell Backend", MB_OK);
				memset(timeBuf, 0, sizeof(timeBuf));
				FormatTime(timeBuf, sizeof(timeBuf));
				printf("[SEND:%s] %s\n", timeBuf, tsk.path);
				fflush(stdout);
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
					length = recv(GLOBAL_SOCKET, tmpBuf, sizeof(tmpBuf), 0);
					tsk.status = TRUE;		// 如果发送成功,标记发送状态为 真
					printf("[SENT-OK:]\n");
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
	char *pEn = getenv("TMP");
	if (pEn)
	{
		strncpy(SYS_TMP_PATH, pEn, MAX_PATH);
		TMPPATH_GOT = TRUE;
		printf("TMP is [%s]\n", SYS_TMP_PATH);
	}
	else
	{
		printf("undefined [TMP]\n");
	}

	SendMsg2Backend();
	return 0;
}

bool FormatTime(char *buffer, int bufSize)
{
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, bufSize, "%H:%M:%S", timeinfo);
	//strftime(buffer, bufSize, "Now is %Y/%m/%d %H:%M:%S", timeinfo);

	return true;
}