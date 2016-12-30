#include <string.h>
#include <iostream>
#include <stdio.h>

#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"
#include "process.h"
#include "../AutoStart.h"
#include "../patches.h"
#include "../PickFiles.h"
#include "mUSB.h"

using namespace std;

#pragma comment(lib,"ws2_32.lib")		// 建立socket()套接字
#pragma comment(lib,"libcrypto.lib")	// ssl 加密函数
#pragma comment(lib,"libssl.lib")		// ssl 安全信道 
#pragma comment(lib, "iphlpapi.lib")	// 获取网络连接状况

#define LOCAL_SCAN

#define CONTROL				0
#define FULL_DEBUG			0
#define DEBUG_PARSE_FILE	0
#define SESSION				1


// 控制本程序主循环
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


// 读取本地扫描结果缓存
vector<HashItem> hashList;

// 关键字列表容器
vector<Keyword> kw;

static string keywordPath = KEYWORD_PATH;
static string hashPath = HASHLST_PATH;

int main(int argc, char *argv[])
{
	// 记录当前的网络连接情况
	vector<Connection> cons;

	// 临时记录日志
	string logMessage;
	// 临时文件信息
	SFile file;
	
	// 用户登录账号
	Account act;

	// 设置终端的隐藏属性:调试不隐藏，正式运行时则隐藏
	// 通过判断启动参数识别出程序当前处于的状态:(调试|正式运行)
	bool hide = false;

	// 如果什么参数也没有，则退出本程序
	if (1 == argc)
	{
		printf("Need args\n");
		return 1;
	}

	// 当该程序自动运行时，默认从注册表中解析出认证信息
	if (2 == argc)
	{
		if (!strncmp(argv[1], "--autostart", 32))
		{
			hide = true;
		}
		else if (!strncmp(argv[1], "--start", 32))	// 从注册表中读取认证信息
		{
			hide = false;
		}
		else
		{
			printf("Unknown Args: [%s]\n", argv[1]);
			exit(1);
		}
		if (!GetAuth(&act))
		{
			printf("[FAILED] GetAuth\n");
			return 1;
		}
	}

	// 得到两个启动参数：用户名、密码
	if (argc == 3)
	{
		// 初始化账户信息
		strcpy_s(act.username, 32, argv[1]);
		strcpy_s(act.password, 32, argv[2]);
	}

	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// 创建一个本地 TCP 端口,与IO过滤中心通信
	string wiredMac;			// 临时获取网卡地址
	char authBuf[128];			// 记录认证消息

	if (!GetWiredMac(wiredMac))
	{
		//cout << "GetWiredMac Error" << endl;
		return 1;
	}

	memset(authBuf, 0, sizeof(authBuf));

	// 构造用户名密码格式,以回车符分割
	sprintf(authBuf, "%s\n%s\n%s", act.username, act.password, wiredMac.c_str());

	string keywords = "keywords.txt";

	InitDir(hide);

	User app(authBuf);
	USB	 usb;

	if (!app.Authentication())  // 验证账号	
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}

	// [本地测试] 控制当前主机与 Internet 的连接、关闭
#if 0
	while (g_RUNNING)
	{
		cout << "paued shut" << endl;
		getchar();
		printf("shutdown network\n");
		RemoteShutdownNetwork(logMessage, string("SHUT"));

		cout << "paued open" << endl;
		getchar();
		RemoteShutdownNetwork(logMessage, string("OPEN"));
	}
	printf("Waiting Thread\n");
	WaitForSingleObject(hThread, INFINITE);
	printf("Done\n");
#else

	// 先留下接口,后期优化时加上此功能---"记录本地敏感文件的哈希缓存" 以提高文件检索速度
	//LoadHashList(hashPath, hashList);


	// 用户手动运行该程序并且登录成功，则刷新认证信息到注册表
	if (3 == argc)
	{
		SetAuth(&act);
	}
	// 每次启动，先更新关键字列表
	app.GetFile(keywords);
	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}

	char localPath[MAX_PATH];	// 临时存储敏感文件路径

	while (g_RUNNING)
	{
		// 检查USB 接口
		if (CheckUsbDevice(usb))
		{
			logMessage = usb.getMessage();
			// cout << logMessage;
			logMessage = GBKToUTF8(usb.getMessage().c_str());
			app.SendLog(NULL, logMessage.c_str(), USB_PLUG_EVENT);
		}

		if (GetInformMessage(localPath, MAX_PATH))
		{
			printf("get new task: %s\n", localPath);
			memset(&file, 0, sizeof(file));
			file.localPath = localPath;

			// 判断是否为涉密文件
			if (fsFilter(file, kw, hashList, logMessage))
			{
				wrapEncreytFile(file);		// 加密涉密文件
				app.UploadFile(file);		// 上传加密后的涉密文件
				cout << "Logmsg: " << logMessage << endl;	// 打印文件关键字匹配详情
				app.SendLog(file.fileHash.c_str(), logMessage.c_str());		// 上传日志
			}
			CleanTmpFiles(file);
		}

		//cout << "Before GetFromServer" << endl;
		app.GetFromServer();   // 接收服务端发送的 远程控制指令
		//cout << "After GetFromServer" << endl;

		app.ExecControl();    // 处理远程控制任务
		app.HeartBeat();	  // 休眠 CLIENT_SLEEP_TIME 毫秒定时向服务端发送一个心跳包

		if (!g_RUNNING)
		{
			app.EndSession();
			break;
		}		
	}

	// 等待线程执行结束
	//WaitForSingleObject(hThread, INFINITE);

#endif

	return 0;
}


// File End.
