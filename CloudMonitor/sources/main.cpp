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
#define SESSION				0


void InitDir(); //patches.cpp

BOOL g_RUNNING = TRUE;

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
	const char* user_name = NULL;
	const char* user_pass = NULL;

	if (argc == 3)
	{
		user_name = argv[1];
		user_pass = argv[2];
	}

	InitDir();
	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}


	// 先留下接口,后期优化时加上此功能---"记录本地敏感文件的哈希缓存" 以提高文件检索速度
	//LoadHashList(hashPath, hashList);


#if CONTROL
	string	netApps;
	if (CheckNetworkApps(plst, netApps))
	{
		cout << netApps << endl;
	}

	//if (!GetProcessList(plst))
	//{
	//	cout << "GetProcessList: empty!" << endl;
	//	return -1;
	//}
	//plst[0].shutdown = true;
	//if (KillProcess(plst))
	//{
	//	GenKillResult(plst, message);
	//	cout << "kill result: [" << message << "]" << endl;
	//}
	//else
	//{
	//	cout << "No Process to kill ..." << endl;
	//}
	//ShowProcessList(plst);
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


	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// 创建一个本地 TCP 端口,接收敏感事件

#if SESSION

	//const char*  user_num = "1234567";
	const char*  user_num = "1237";
	if (NULL != user_name)
	{
		user_num = user_name;
	}

	User app(user_num);
	
	if (!app.Authentication())  // 验证账号	
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}

	char localPath[MAX_PATH];	// 临时存储敏感文件路径

	string	netApps;
	while (g_RUNNING)
	{
		//cout << "while looping ..." << endl;
		if (GetInformMessage(localPath, MAX_PATH))
		{
			// 查看当前是否有网络进程在运行
			if (CheckNetworkApps(plst, netApps))
			{
				memset(&file, 0, sizeof(file));
				file.localPath = localPath;

				// 判断是否为涉密文件
				if (fsFilter(file, kw, hashList, logMessage))
				{
					app.UploadFile(file);
					logMessage += netApps;
					app.SendLog(file.fileHash.c_str(), logMessage.c_str());
				}
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

	if (NULL != hThread)
	{
		WaitForSingleObject(hThread, INFINITE);  // wait
	}

	return 0;
}


// File End.