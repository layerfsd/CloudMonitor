#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"
#include "process.h"


#include <iostream>

using namespace std;

#pragma comment(lib,"ws2_32.lib")		// 建立socket()套接字
#pragma comment(lib,"libcrypto.lib")	// ssl 加密函数
#pragma comment(lib,"libssl.lib")		// ssl 安全信道 
#pragma comment(lib, "iphlpapi.lib")	// 获取网络连接状况


#define CONTROL				0
#define FULL_DEBUG			0
#define DEBUG_PARSE_FILE	0
#define SESSION				1

BOOL g_RUNNING = TRUE;
BOOL DeleteDirectory(const char * DirName);

inline void InitDir()
{

	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "//..//");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectory(strModule);
	GetCurrentDirectory(sizeof(strModule), strModule);

	cout << "Working Dir: " << strModule << endl;
	DeleteDirectory(TMP_DIR);
	return;
}



// Use signal to attach a signal handler to the abort routine
#include <signal.h>
#include <tchar.h>

void SignalHandler(int signal)
{
	printf("\nExciting...\n");
	g_RUNNING = FALSE;
	return;
}

void regSINGINT()
{
	typedef void(*SignalHandlerPointer)(int);

	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGINT, SignalHandler);

	return ;
}


int main(int argc, char *argv[])
{
	string keywordPath = KEYWORD_PATH;
	string hashPath = HASHLST_PATH;
	string sensiFilePath;
	string logMessage;

	vector<Keyword> kw;
	vector<Connection> cons;
	vector<Service> KeyPorts;
	vector<HashItem> hashList;
	vector<Process> plst;

	SFile file;

	regSINGINT(); //注册 CTRL+C 信号处理函,正常终止会话.
	InitDir();
	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}


	// 先留下接口,后期优化时加上此功能---"记录本地敏感文件的哈希缓存" 以提高文件检索速度
	//LoadHashList(hashPath, hashList);


#if CONTROL
	string	message;
	if (!GetProcessList(plst))
	{
		cout << "GetProcessList: empty!" << endl;
		return -1;
	}
	//plst[0].shutdown = true;
	if (KillProcess(plst))
	{
		GenKillResult(plst, message);
		cout << "kill result: [" << message << "]" << endl;
	}
	else
	{
		cout << "No Process to kill ..." << endl;
	}
	ShowProcessList(plst);
#endif

#if DEBUG_PARSE_FILE
	char *fList[] = {
		"F:\\NutStore\\SSL传输\\ClientPython2CPP.txt",
		"F:\\NutStore\\SSL传输\\安全办公信息监控平台项目研发方案20160922.docx",
		"F:\\NutStore\\SSL传输\\软件.doc"
	};
	int len = sizeof(fList) / sizeof(fList[0]);
	for (int i = 0; i < len; i++)
	{
		memset(&file, 0, sizeof(file));
		file.localPath = fList[i];
		int ret = fsFilter(file, kw, hashList, logMessage);
	}
#endif

#if FULL_DEBUG
	char mac[32];
	vector<Connection> cons;
	vector<Service> KeyPorts;

	cout << "Getting MAC address ..." << endl;
	if (!GetMac(mac))
		cout << mac << endl;

	// -1 error, 0 Ok
	if (!IsCnt2Internet())
		cout << "Connect to Internet OK..." << endl;
	else
		cout << "Connect to Internet Failed!" << endl;
	
	
	//ParseText("test.docx", "test.txt");
	//NetStat();
	GetConnections(cons);
	ReadKeyPorts("res/tcpList.txt", KeyPorts);

	//DisplayPorts(KeyPorts);
	int ret = CheckKeyConnections(cons, KeyPorts);
	ShowKeyConnections(cons, ret);
#endif


#if SESSION


	char*  user_num = "1234567";
	
	User app(user_num);
	
	if (!app.Authentication())  // 验证账号	
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}

	CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// 创建一个本地 TCP 端口,接收敏感事件
	char localPath[MAX_PATH];	// 临时存储敏感文件路径

	while (g_RUNNING)
	{
		//cout << "while looping ..." << endl;
		if (GetInformMessage(localPath, MAX_PATH))
		{
			memset(&file, 0, sizeof(file));
			file.localPath = localPath;
			if (fsFilter(file, kw, hashList, logMessage))
			{
				cout << "logMessag: " << logMessage << endl;
				app.UploadFile(file);
				app.SendLog(file.fileHash.c_str(), FILE_NETEDIT, logMessage.c_str());
			}
		}

		//cout << "No message from >>>Local ..." << endl;
		app.GetFromServer();   // 接收服务端发送的 远程控制指令
		app.ExecControl();    // 处理远程控制任务
		app.HeartBeat();	  // 休眠 CLIENT_SLEEP_TIME 毫秒定时向服务端发送一个心跳包

		if (!g_RUNNING)
		{
			app.EndSession();
			break;
		}
		if (app.isEndSession())  //检测服务端是否发出 "终止会话"命令
		{
			break;
		}
	}

#endif // Session

	return 0;
}


// File End.