#include "tools.h"
#include "crc32.h"
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <time.h>
#include <sys/types.h>  
#include <sys/stat.h>  
#include <windows.h>

#define MAX_QUEUE_SIZE 1024
#define SEM_NAME	"ALBERT_SYNC"
#define SERV_ADDR   "127.0.0.1"
#define SERV_PORT	50006
#define ONE_SECOND	1000
#define MIN_SENT_INTERVAL 3  // 最短发送间隔时间

using namespace std;

struct TASK
{
	CHAR	path[MAX_PATH];
	size_t  ltime;
	unsigned long  crc;
	DWORD	len;
	BOOL    status;
};

#pragma data_seg("GV_ALBERT_QUEUE")
size_t gll_head = 0;
size_t gll_tail = 0;
TASK gll_queue[MAX_QUEUE_SIZE] {0};
#pragma data_seg()
#pragma comment(linker,"/SECTION:GV_ALBERT_QUEUE,RWS")

static BOOL isShutdownNetwork = FALSE;


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



static unsigned long Crc32_ComputeBuf(unsigned long inCrc32, const void *buf, size_t bufLen) {
	static const unsigned long crcTable[256] = {
		0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,
		0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,
		0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,
		0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,
		0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,
		0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
		0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,
		0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
		0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,
		0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,
		0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,
		0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
		0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
		0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,
		0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,
		0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
		0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,
		0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
		0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,
		0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,
		0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,
		0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,
		0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,
		0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
		0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,
		0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
		0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,
		0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,
		0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,
		0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
		0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,
		0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
		0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,
		0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,
		0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,
		0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
		0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
	};
	unsigned long crc32;
	unsigned char *byteBuf;
	size_t i;

	/** accumulate crc32 for buffer **/
	crc32 = inCrc32 ^ 0xFFFFFFFF;
	byteBuf = (unsigned char*)buf;
	for (i = 0; i < bufLen; i++) {
		crc32 = (crc32 >> 8) ^ crcTable[(crc32 ^ byteBuf[i]) & 0xFF];
	}
	return crc32 ^ 0xFFFFFFFF;
}

/*----------------------------------------------------------------------------*\
*  NAME:
*     Crc32_ComputeFile() - compute CRC-32 value for a file
*  DESCRIPTION:
*     Computes the CRC-32 value for an opened file.
*  ARGUMENTS:
*     outCrc32 - (out) result CRC-32 value
*  RETURNS:
*     err - 0 on success or -1 on error
*  ERRORS:
*     - file errors
\*----------------------------------------------------------------------------*/

int Crc32_ComputeFile(const char* fileName, unsigned long *outCrc32) {
	unsigned char buf[CRC_BUFFER_SIZE];
	size_t bufLen;
	FILE *file = NULL;

	if ((file = fopen(fileName, "rb")) == NULL)
	{
		return -1;
	}

	/** accumulate crc32 from file **/
	*outCrc32 = 0;
	while (1) {
		bufLen = fread(buf, 1, CRC_BUFFER_SIZE, file);
		if (bufLen == 0) {
			if (ferror(file)) {
				fprintf(stderr, "error reading file\n");
				return -1;
			}
			break;
		}
		*outCrc32 = Crc32_ComputeBuf(*outCrc32, buf, bufLen);
	}
	return 0;
}

int GetFileSize(const char *FileName, size_t *FileSize)
{
	FILE *fp;

	if ((fp = fopen(FileName, "rb")) == NULL)
	{
		//perror(FileName);  //调试,显示具体出错信息
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	*FileSize = ftell(fp);

	fclose(fp);
	fp = NULL;

	return 0;
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

	WriteToLog("connecting to master");
	return ret;
}


// CreateFile过滤模块采用 TCP 本地通信模式
// 当通信另外一端未启动或者网络错误时,宿主程序会卡在 CreateFile() 系统调用中
// 因此在本地服务中创建一个·命名管道·以缓存需要通知的事件
// 其工作原理是: 接收到 Hook 通知后,将·路径·记录到队列中立刻返回
// 由一个专有线程负责从队列中读取数据通知另外一个程序

static 	map<string, TASK> LastTask;

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
	size_t head = gll_head;
	size_t tail = gll_tail;
	ReleaseSemaphore(semhd, 1, NULL);

	if (head == tail)
	{
		 //printf("end queue\n");
		//ReleaseSemaphore(semhd, 1, NULL);
		// release lock: gll_tail
		return bRet;
	}
	//printf("<<<-------------ReleaseSemaphore\n");

	DWORD nxtPos = 0;

	bRet = FALSE;
	//printf("IN FOR\n");


	for (; (bRet != TRUE) && (head != tail); )
	{
		memset(&cur, 0, sizeof(TASK));
		memset(&nxt, 0, sizeof(TASK));

		nxtPos = (head + 1) % MAX_QUEUE_SIZE;

		cur = gll_queue[head];
		nxt = gll_queue[nxtPos];
		head = nxtPos;

		// 如果当前路径曾经发送过，判断其文件大小是否发生了变化
		if (LastTask.count(cur.path))
		{
			if ( (cur.ltime - LastTask[cur.path].ltime > 1) && 0 == Crc32_ComputeFile(cur.path, &cur.crc) && (cur.crc != LastTask[cur.path].crc))
			{
				printf("[DIFF-CRC] %s %lu %Iu\n", cur.path, cur.crc, cur.ltime);
				bRet = TRUE;
			}
		}
		else
		{
			// 当前路径尚未发送过,计算文件crc
			if (0 == Crc32_ComputeFile(cur.path, &cur.crc))
			{
				printf("[FILE] %s %lu %Iu\n", cur.path, cur.crc, cur.ltime);
				bRet = TRUE;
			}
		}
	}

	//printf("IGNORE [%d:%d] tail: *%s*\n", head, tail, gll_queue[tail].path);

	// 更新游标
	gll_head = head;

	if (bRet)
	{
		memset(tsk, 0, sizeof(TASK));
		*tsk = cur;
		LastTask[cur.path] = cur;
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
	tTask.crc = 0;
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
		if (!_stricmp(pos, matchList[i]))
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
				WriteToLog("Connect to Local Failed\n");
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
		
		if (GetTask(&tsk))
		{
			int   MaxRetryTime = 0;
			while (KEEP_RUNNING && (tsk.status != TRUE) && (MaxRetryTime++ < MAX_RETRY_TIME))
			{
				//MessageBox(NULL, tPath, "Tell Backend", MB_OK);
				memset(timeBuf, 0, sizeof(timeBuf));
				FormatTime(timeBuf, sizeof(timeBuf));
				//printf("[SEND:%s] %s\n", timeBuf, tsk.path);
				WriteToLog(tsk.path);		
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
					//printf("[SENT-OK:]\n");
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


// 不定参数的 HOOK 日志记录
static int WriteToLog(char* fmt, ...)
{
	static const char *appName = "HOOK";

	static char LogFile[] = "Service.txt";
	static char timeBuf[32];
	static char logBuf[256];


	// 得到日志内容
	va_list args;
	va_start(args, fmt);

	memset(logBuf, 0, sizeof(logBuf));
	snprintf(logBuf, sizeof(logBuf), fmt, args);
	
	va_end(args);

	// 获取当前时间
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);

	// 拼接日志时间
	memset(timeBuf, 0, sizeof(timeBuf));
	snprintf(timeBuf, sizeof(timeBuf), "[%d-%02d-%02d %02d:%02d:%02d] %s ", \
		1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, appName);


	FILE* log;
	fopen_s(&log, LogFile, "a+");

	if (log == NULL)
		return -1;

	// 打印 当前时间，日志内容到日志文件中
	fprintf(log, "%s%s\n", timeBuf, logBuf);

	fclose(log);
	return 0;
}
