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
#define MIN_SENT_INTERVAL 3  // ��̷��ͼ��ʱ��

BOOL		KEEP_RUNNING = TRUE;
BOOL		isConnectionOK = FALSE;
SOCKET      GLOBAL_SOCKET = { 0 };

#define TRASH_FILE   "$RECYCLE.BIN"

static CHAR SYS_TMP_PATH[MAX_PATH];
static BOOL TMPPATH_GOT = false;

#pragma comment(lib,"ws2_32.lib")		// ����socket()�׽���


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

	// ��ǰʱ�� - ����ʱ�� > MIN_CREATETIME
	if (Curtime - (size_t)buf.st_ctime > MIN_CREATETIME)
	{
		return true;
	}
	// ���ļ�Ϊ�´����Ŀհ��ļ�,������
	return false;
}



// ͨ���� TCP �ӿ�ͨ����Ϣ
// ��ʼ�� TCP ����,������Ҫ����˫�������֤����
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


// CreateFile����ģ����� TCP ����ͨ��ģʽ
// ��ͨ������һ��δ���������������ʱ,��������Ῠ�� CreateFile() ϵͳ������
// ����ڱ��ط����д���һ���������ܵ����Ի�����Ҫ֪ͨ���¼�
// �乤��ԭ����: ���յ� Hook ֪ͨ��,����·������¼�����������̷���
// ��һ��ר���̸߳���Ӷ����ж�ȡ����֪ͨ����һ������

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


		// �����ǰ·������һ��·����ȣ���������ǰ·��
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
	
	if (bRet)	// �ɹ���ȡ������
	{
		// ��һ��ʱ����,�������ظ�����.
		// ����Ƿ�����һ������һ��

		// ����·���Ƿ��� MIN_SENT_INTERVAL �����ظ����͹�
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

	// ��������
	WaitForSingleObject(semhd, INFINITE);
	if (gll_head == (gll_tail + 1) % MAX_QUEUE_SIZE)
	{
		ReleaseSemaphore(semhd, 1, NULL);
		return;
	}
	ReleaseSemaphore(semhd, 1, NULL);

	// �ַ������ȷǷ�
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


// ��� tcp ����Ŀ��IP �Ƿ���Ϲ淶
BOOL CheckSockAddr(const struct sockaddr FAR *saddr)
{
	// ������ر��û����磬��ֱ�ӷ���TRUE
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
	// ��������ˡ�������ơ������н���Ҫ��������ʱ�������Ի���
	//if (FALSE == bRet)
	//{
	//	char *strAddr;
	//	strAddr = inet_ntoa(s->sin_addr);
	//	char strModule[MAX_PATH];
	//	GetModuleFileName(NULL, strModule, MAX_PATH); //�õ���ǰģ��·��

	//	MessageBox(NULL, strAddr, strModule, MB_OK);
	//}

	return bRet;
}



// �ж��Ƿ�Ϊ ָ����ʽ���ļ�
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
	//����ȡ�ļ���׺
	LPCSTR pos = strrchr(lpFilePath, '.');

	if (NULL == pos || '\\' == lpFilePath[0])
	{
		return FALSE;
	}

	if (NULL != strchr(lpFilePath, '$'))
	{
		return FALSE;
	}

	//// ����ϵͳ��ʱ�ļ�
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

// �뱾�ؽ��� CloudMonitor.exe Э��ִ��Զ�̵Ŀ���ָ��
// �� 100 ��ʼ��������900 ��1000-100=900��������ָ��
// ����Ĭ�ϡ�0����ʼ������ 10��0-9��������ָ���������չ
enum {
	CMD_GOT = 100,	// ȷ����Ϣ
	STOP_SERVICE,
	OPEN_NETWORK,
	SHUT_NETWORK
}LOCAL_CONTROL;

static const int LocalControlNumLen = 3;

// �������� CloudMonitor.exe ��ָ��
void CheckTaskFromLocal(SOCKET sock)
{
	static	FD_SET fdRead;
	static  char   tmpBuf[128];
	static  char reply[] = "Yes Commander.";

	int		nRet = 0;		//��¼���ͻ��߽��ܵ��ֽ���
	static int	CMD = 0;


	memset(tmpBuf, 0, sizeof(tmpBuf));
	nRet = recv(GLOBAL_SOCKET, tmpBuf, LocalControlNumLen, 0);
	DWORD	netAddr;

	if (nRet > 0)
	{
		CMD = 0;

		// ���ַ�����ʽ������ת��Ϊ����
		tmpBuf[LocalControlNumLen] = 0;
		CMD = atoi(tmpBuf);
	
		if (STOP_SERVICE == CMD)			// �رյ�ǰ����
		{
			printf("[CMD] STOP SERVICE\n");
			KEEP_RUNNING = FALSE;
		}
		else if (OPEN_NETWORK == CMD)		// �����ͻ����� Internet ������ͨ��
		{
			printf("[OPEN NETWORK]\n");
			isShutdownNetwork = FALSE;
		}
		else if (SHUT_NETWORK == CMD)	// �رտͻ����� Internet ������ͨ��,������ ��Զ�̷�������������
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



// ���̨������һ����Ϣ
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

		if (loopCount >= 3)	// �˴���һ��������������֤��CloudMonitor������ͨ�š������3���뷢��һ��
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
			// ���ͱ��ء��������������۳ɹ���񣬶���������Ĵ���
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
					// �������ʧ�ܣ���������״��Ϊ:�ѶϿ�����
					if (sent < 0)  // TCP����ʧЧ
					{
						printf("[SENT-FAILED:]\n");
						isConnectionOK = FALSE;
					}
				}
				else  // ���ͳɹ�
				{
					length = recv(GLOBAL_SOCKET, tmpBuf, sizeof(tmpBuf), 0);
					tsk.status = TRUE;		// ������ͳɹ�,��Ƿ���״̬Ϊ ��
					printf("[SENT-OK:]\n");
				}

				if (!tsk.status)
				{
					// ������������֮ǰ���ȵȴ�һ���
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