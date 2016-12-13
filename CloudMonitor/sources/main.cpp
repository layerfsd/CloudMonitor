#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"
#include "process.h"

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


	// 先留下接口,后期优化时加上此功能---"记录本地敏感文件的哈希缓存" 以提高文件检索速度
	//LoadHashList(hashPath, hashList);


	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// 创建一个本地 TCP 端口,接收敏感事件


	const char*  user_num = "1234567";
	char  authBuf[64];
	memset(authBuf, 0, sizeof(authBuf));

	// 构造用户名密码格式,以回车符分割
	sprintf(authBuf, "%s\n%s", user_name, user_pass);
	//const char*  user_num = "1234568";
	if (NULL != user_name)
	{
		user_num = user_name;
	}

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