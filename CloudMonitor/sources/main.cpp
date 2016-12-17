#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"
#include "process.h"
#include "../AutoStart.h"

#include <string.h>
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


void InitDir();					//patches.cpp

BOOL g_RUNNING = TRUE;


// 删除临时文件
void CleanTmpFiles(SFile& file)
{
	remove(file.savedPath.c_str());		// tmp\doc
	remove(file.encPath.c_str());		// tmp\aes
	remove(file.txtPath.c_str());		// tmp\txt
}

// 获取本机有线网卡地址
bool GetWiredMac(string& wiredMac);


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
	Account act;

	// 当该程序自动运行时，默认从注册表中解析出认证信息
	if (2 == argc && !strncmp(argv[1], "--autostart", 32))
	{
		if (!GetAuth(&act))
		{
			printf("[FAILED] GetAuth\n");
			exit(1);
		}
	}
	// 用户手动运行该程序，则刷新认证信息到注册表
	if (argc == 3)
	{
		// 初始化账户信息
		strcpy_s(act.username, 32, argv[1]);
		strcpy_s(act.password, 32, argv[2]);
		SetAuth(&act);
	}

	InitDir();


	// 先留下接口,后期优化时加上此功能---"记录本地敏感文件的哈希缓存" 以提高文件检索速度
	//LoadHashList(hashPath, hashList);


	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// 创建一个本地 TCP 端口,接收敏感事件


	char  authBuf[128];
	memset(authBuf, 0, sizeof(authBuf));

	string wiredMac;

	if (!GetWiredMac(wiredMac))
	{
		cout << "GetWiredMac Error" << endl;
		exit(1);
	}
	// 构造用户名密码格式,以回车符分割
	sprintf(authBuf, "%s\n%s\n%s", act.username, act.password, wiredMac.c_str());
	//const char*  user_num = "1234568";

	char localPath[MAX_PATH];	// 临时存储敏感文件路径

	string	netApps;

	string keywords = "keywords.txt";
	

#if 0
	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}

	while (g_RUNNING)
	{
		if (GetInformMessage(localPath, MAX_PATH))
		{
			memset(&file, 0, sizeof(file));
			file.localPath = localPath;

			// 判断是否为涉密文件
			if (fsFilter(file, kw, hashList, logMessage))
			{
				cout << "Logmsg: " << logMessage << endl;
			}
			CleanTmpFiles(file);
		}
	}
#else
	User app(authBuf);

	if (!app.Authentication())  // 验证账号	
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}

	// 每次启动，先更新关键字列表
	app.GetFile(keywords);
	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}

	while (g_RUNNING)
	{
		if (GetInformMessage(localPath, MAX_PATH))
		{
			memset(&file, 0, sizeof(file));
			file.localPath = localPath;

			// 判断是否为涉密文件
			if (fsFilter(file, kw, hashList, logMessage))
			{
				app.UploadFile(file);
				cout << "Logmsg: " << logMessage << endl;
				app.SendLog(file.fileHash.c_str(), logMessage.c_str());
			}
			CleanTmpFiles(file);
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
	}
#endif

	return 0;
}


// File End.