// tmpFunc.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#include <queue>

using namespace std;



#define  MAXBUF 256
//  创建一个队列,用来缓存 路径列表
static queue<string> LocalPathList;



static SOCKET GLOBALclntSock;

static HANDLE  hNamedPipe;
static bool    Accept = false;



void FileFunc()
{
	printf("FileFunc\n");
}
void EditFunc()
{
	printf("EditFunc\n");
}

void testFunc1()
{
	typedef void(*funcp)();
	funcp pfun = FileFunc;

	pfun();
	pfun = EditFunc;
	pfun();
}

int testAtoi()
{
	char s[] = "001";
	char s2[] = "0010";

	printf("strs: %s %d\n", s, atoi(s));
	printf("strs: %s %d\n", s2, atoi(s2));
	return 0;
}


//注意：当字符串为空时，也会返回一个空字符串  
void split(std::string& s, std::string& delim, std::vector< std::string >* ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		ret->push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last>0)
	{
		ret->push_back(s.substr(last, index - last));
	}
}



bool parseProcessSeq(string& args)
{
	int			count = 0;
	int			pos = 0;

	int*    	seqList = NULL;
	char*		largs = NULL;

	char*		token = NULL;

	largs = new char[args.length() + 1];

	if (NULL == largs)
	{
		cout << "failed new char " << args.length() << endl;
		return false;
	}
	strcpy(largs, args.c_str());


	token = strtok(largs, "\n");
	count = atoi(token);	//获取要终结进程的总数
	cout << "Count: " << count << endl;

	seqList = new int[count];

	if (NULL == largs)
	{
		cout << "failed new int " << count << endl;
		return false;
	}

	while (NULL != token && pos < count)
	{
		token = strtok(NULL, "\n");
		seqList[pos] = atoi(token);
		cout << "pos: " << pos << " seq: " << seqList[pos] << endl;
		pos += 1;
	}


	delete[] seqList;
	delete[] largs;
	return false;
}


bool parseProcessSeq(string& args, vector<int>& seqList)
{
	int		count = 0;
	char*		largs = NULL;
	char*		token = NULL;

	largs = new char[args.length() + 1];

	if (NULL == largs)
	{
		cout << "failed new char " << args.length() << endl;
		return false;
	}
	strcpy(largs, args.c_str());


	token = strtok(largs, "\n");
	count = atoi(token);	//获取要终结进程的总数
	cout << "Count: " << count << endl;


	if (NULL == largs)
	{
		cout << "failed new int " << count << endl;
		return false;
	}

	int			pos = 0;
	int			tp = 0;
	while (NULL != token && pos < count)
	{
		token = strtok(NULL, "\n");

		tp = atoi(token);
		seqList.push_back(tp);
		cout << "pos: " << pos << " seq: " << seqList[pos] << endl;
		pos += 1;
	}


	delete[] largs;
	return true;
}

void testParseProcessSeq()
{
	string a = "3\n55\n66\n77\n";
	vector<int> seqList;

	parseProcessSeq(a, seqList);

	for (size_t i = 0; i < seqList.size(); i++)
	{
		printf("i: %d seq: %d\n", i, seqList[i]);
	}

}


void testDoubleIfinFor()
{
	for (int i = 0; i < 3; i++)
	{
		if (i > 1)
		{
			cout << "i: " << i << " first if" << endl;
			if (i > 1)
			{
				cout << "i: " << i << " second if" << endl;
				break;
			}
			cout << "i: " << i << "after second if" << endl;
		}
	}
	return;
}



#include <stdio.h>



#pragma comment(lib,"ws2_32.lib")		// 建立socket()套接字

//#define SERV_ADDR  "192.168.43.132"
#define SERV_ADDR  "127.0.0.1"
#define SERV_PORT	50006

static SOCKET GLOBAL_SOCKET;
static BOOL	  isConnectionOK = FALSE;


inline BOOL InitTcpConnection()
{
	if (isConnectionOK)
	{
		printf("[OK] Connected to %s:%d\n", SERV_ADDR, SERV_PORT);
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


inline BOOL SendMsg2Backend(LPCSTR lpStrs)
{
	DWORD length = strlen(lpStrs) + 1;

	if (!isConnectionOK)
	{
		if (!InitTcpConnection())
		{
			printf("[ERROR] Not Connected to %s:%d\n", SERV_ADDR, SERV_PORT);
			return FALSE;
		}
	}

	if (send(GLOBAL_SOCKET, lpStrs, length, 0) <= 0)
	{
		printf("Error!\n");
	}
	else
	{
		printf("send [%s] OK\n", lpStrs);
		CHAR  tmpBuf[MAXBYTE];
		// 经实际测试发现,如果不接受后台返回数据会影响双方正常通信.
		memset(tmpBuf, 0, sizeof(tmpBuf));
		int len = recv(GLOBAL_SOCKET, tmpBuf, sizeof(tmpBuf), 0);
		printf("replied: [%d] [%s]\n", len, tmpBuf);
	}
	return TRUE;
}


inline BOOL ProcessFilePath(LPCSTR lpFilePath)
{
	static LPCSTR matchList[] = {
		"doc",
		"docx",
		"txt",
		"text"
	};

	static DWORD dwMatchListLen = sizeof(matchList) / sizeof(matchList[0]);

	if (NULL == lpFilePath)
	{
		return FALSE;
	}

	LPCSTR pos = strrchr(lpFilePath, '.');

	if (NULL == pos)
	{
		return FALSE;
	}

	BOOL retValue = FALSE;

	for (DWORD i = 0; i < dwMatchListLen; i++)
	{
		if (!strcmp(pos + 1, matchList[i]))
		{
			retValue = TRUE;
			//SendMsg2Backend(lpFilePath);
			break;
		}
	}

	return retValue;
}


void testProcessFilePath()
{
	char szBuf[MAXBYTE] = "d:\\test.docx";
	if (ProcessFilePath(szBuf))
	{
		printf("%s match\n", szBuf);
	}
}






int InitTcp()
{
	SOCKET serversoc;
	SOCKADDR_IN serveraddr;
	SOCKADDR_IN clientaddr;
	int len;

	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);	//initial Ws2_32.dll by a process
	if ((serversoc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)	//create a tcp socket
	{
		printf("Create socket fail!\n");
		return -1;
	}

	//	int iResult;
	//	u_long iMode = 0;
	//	iResult = ioctlsocket(serversoc, FIONBIO, &iMode);
	//	if (iResult != NO_ERROR)
	//	{
	//		printf("ioctlsocket failed with error: %d\n", iResult);
	//	}


	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(50006);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(serversoc, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) != 0)
	{
		printf("Bind fail!\n");
		return -1;
	}

	//start listen, maximum length of the queue of pending connections is 1
	printf("Start listen...\n");
	if (listen(serversoc, 1) != 0)
	{
		printf("Listen fail!\n");
		return -1;
	}

	len = sizeof(SOCKADDR_IN);
	char tPath[MAX_PATH];

RESTART_LISTEN:
	while (1)
	{
		//waiting for connecting
		if ((GLOBALclntSock = accept(serversoc, (SOCKADDR *)&clientaddr, &len)) <= 0)
		{
			printf("Accept fail!\n");
			goto RESTART_LISTEN;
		}

		printf("Connected\n");
		Accept = true;
		while (Accept)
		{
			int ret = 0;

			memset(tPath, 0, sizeof(tPath));
			ret = recv(GLOBALclntSock, tPath, MAXBUF, 0);


			if (ret > 0)		//仅当成功接收,才把信息加入缓冲队列
			{
				int sent = send(GLOBALclntSock, tPath, ret, 0);
				cout << "sent: " << sent << "bytes" << endl;
				string tmp = tPath;
				LocalPathList.push(tPath);
				cout << "Get: " << tPath << endl;
			}
			else
			{
				Accept = false;
			}

		}// inner while(Accept)
	}// outter while(1)
	return 0;

}

void FreeTcp()
{
	return;
}


#pragma comment(lib,"ws2_32.lib")		// 建立socket()套接字


#include <comdef.h>

void testWchar()
{
	const WCHAR* wc = L"Hello World";
	_bstr_t b(wc);
	const char* c = b;

	printf("output: %s\n", c);
}




int testAES();

BOOL DeleteDirectory(const char * DirName);




// Use signal to attach a signal handler to the abort routine
#include <signal.h>
#include <tchar.h>

void SignalHandler(int signal)
{
	printf("\nExciting...\n");
	exit(signal);
}

int testSignal()
{
	typedef void(*SignalHandlerPointer)(int);

	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGINT, SignalHandler);

	while (1)
	{
		printf("working \n");
		Sleep(700);
	}
	return 1;
}



int testUDP();
void InitUDP();
int  SendMsg(char* sendData);
void EndUDP();
int tcpClient();

BOOL GetProcessList();

// 程序入口
int call();


void testMemcmp()
{
	char str1[] = "hello";
	char str2[] = "hello Albert";
	cout << memcmp(str1, str2, strlen(str1)) << endl;
	cout << strncmp(str1, str2, strlen(str2)) << endl;
}


bool GetMyName(char* szBuf, size_t bufSize)
{

	CHAR    szPath[MAX_PATH] = { 0 };

	if (!GetModuleFileNameA(NULL, szPath, MAX_PATH))
	{
		printf("GetModuleFileName failed (%d)\n", GetLastError());
		return false;
	}

	char* pos = NULL;

	pos = strrchr(szPath, '\\');

	if (NULL == pos)
	{
		return false;
	}

	strncpy(szBuf, pos + 1, bufSize);
	return true;
}


void testMutex()
{
	char	sem_name[MAX_PATH];

	memset(sem_name, 0, MAX_PATH);
	if (!GetMyName(sem_name, MAX_PATH))
	{
		return;
	}
	printf("sem_name: %s\n", sem_name);

	HANDLE  semhd = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, FALSE, sem_name);
	if (NULL == semhd)
	{
		printf("First Startup!\n");
		semhd = CreateSemaphoreA(NULL, 1, 1, sem_name);
		if (NULL == semhd)
		{
			printf("Create [%s] failed.\n", sem_name);
			return;
		}
		while (1)
		{
			Sleep(200);
		}
	}
	else
	{
		printf("I am already running ...\n");
		CloseHandle(semhd);
	}
}

void testSendText();

void testNamePipe();
void testClient();

string FormatMAC();

int main()
{
	//testParseProcessSeq();
	//testDoubleIfinFor();

	//itTcp();  //tcp server listen on localhost:50006
	//testMutex();
	//testMemcmp();

	//testSendText();
	//testNamePipe();
	string t = "F8:2F:A8:F2:E7:51";
	cout << t.substr(0, 9) << endl;
	cout << "mac: " << FormatMAC() << endl;

	//GetProcessList();

//	testProcessFilePath();
	//call();
	//tcpClient();

	//testProcessFilePath();
	//cout << a << " size: " << a.size() << endl;
	//cout << a << " length: " << a.length() << endl;

	//testWchar();
	//testAES();
	//testAES2();
	return 0;
}