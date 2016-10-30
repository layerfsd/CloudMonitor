#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"

#include <iostream>

using namespace std;

#pragma comment(lib,"ws2_32.lib")		// 建立socket()套接字
#pragma comment(lib,"libcrypto.lib")	// ssl 加密函数
#pragma comment(lib,"libssl.lib")		// ssl 安全信道
#pragma comment(lib, "iphlpapi.lib")	// 获取网络连接状况

#define FULL_DEBUG			0
#define DEBUG_PARSE_FILE	1
#define SESSION				0


inline void InitDir()
{

	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径
	cout << strModule << endl;

	strcat(strModule, "//..//");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectory(strModule);
	GetCurrentDirectory(sizeof(strModule), strModule);
	return;
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

	SFile file;

	InitDir();

#if DEBUG_PARSE_FILE
	file.localPath = "F:\\NutStore\\SSL传输\\ClientPython2CPP.txt";
	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}
	int ret = fsFilter(file, kw, hashList, logMessage);
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


//	char*  user_num = "3130931002";
	char*  user_num = "1234567";

	if (0 != InitSSL(SERV_ADDR, SERV_PORT))
	{
		return -1;
	}
	
	User app(user_num);
	// 验证账号	
	if (!app.Authentication())
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}


	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}
	// 先留下接口,后期优化时加上此功能---本地敏感文件的哈希缓存以提高文件检索速度
	//LoadHashList(hashPath, hashList);


	//cout << "CreateNamedPipeInServer..." << endl;
	CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
	//cout << "CreateNamedPipeInServer Success" << endl;


#if 1
	char lpath[MAX_PATH];
	int cnt = 0;
	while (true)
	{
		//cout << "while looping ..." << endl;
		//if (!GetNamedPipeMessage(lpath))
		if (GetInformMessage(lpath, MAX_PATH))
		{
			memset(&file, 0, sizeof(file));
			file.localPath = lpath;
			if (fsFilter(file, kw, hashList, logMessage))
			{
				cout << "logMessag: " << logMessage << endl;
				app.UploadFile(file);
				app.SendLog(file.fileName.c_str(), FILE_NETEDIT, logMessage.c_str());
			}
		}
		else
		{
			//cout << "No message from NamePipe..." << endl;
		}



		cnt += 1;
		if (cnt >= 1000)
		{
			cnt = 0;
			app.HeartBeat();
		}
		Sleep(50);
	}
#endif
	//app.GetFile(string("keywords.txt"));
	app.EndSession();

	EndSSL();

#endif // Session

	return 0;
}


// File End.