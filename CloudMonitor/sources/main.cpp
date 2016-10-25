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
#define SESSION				1

int main(int argc, char *argv[])
{
	string keywordPath = KEYWORD_PATH;
	string hashPath = HASHLST_PATH;
	string sensiFilePath;
	string logMessage;

	vector<Keyword> kw;
	vector<Connection> cons;
	vector<Service> KeyPorts;
	// map<filePath, fileHash>
	vector<HashItem> hashList;

	SFile file;


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


#if DEBUG_PARSE_FILE
	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
	}
	// 先留下接口,后期优化时加上此功能---本地敏感文件的哈希缓存以提高文件检索速度
	//LoadHashList(hashPath, hashList);
	//memset(&file, 0, sizeof(file));
	//file.localPath = "C:\\Users\\tiny\\Downloads\\题库.doc";
	//fsFilter(file, kw, hashList, logMessage);

	memset(&file, 0, sizeof(file));
	file.localPath = "C:\\Users\\tiny\\Downloads\\安全办公信息监控平台项目研发方案20160922.docx";
	fsFilter(file, kw, hashList, logMessage);
	cout << "logMessag: " << logMessage << endl;
#endif

#if SESSION


	char*  user_num = "3130931002";

	InitSSL(SERV_ADDR, SERV_PORT);
	
	User app(user_num);
	
	if (!app.Authentication())
	{
		cout << "Auth Failed!" << endl;
		return 1;
	}
	app.SendLog(file.fileName.c_str(), FILE_NETEDIT, logMessage.c_str());

	app.GetFile(string("keywords.txt"));
	app.UploadFile(file);
	app.EndSession();

	EndSSL();

#endif // Session
	//cout << "Say something: ";
	//cin >> message;

	return 0;
}


// File End.