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
	vector<Keyword> kw;
	string message;

	char docName[] = "res\\safe.doc";
	char *pureName = strrchr(docName, '\\')+1;
	char genName[] = "safe.txt";
	char keywordFile[] = "res\\keywords.txt";

	ReadKeywords(keywordFile, kw);
	ParseFile2Text(docName, genName);
	KeywordFilter(kw, genName, message);
	cout << message << endl;
#endif

#if SESSION


	char*  user_num = "3130931001";

	InitSSL(SERV_ADDR, SERV_PORT);
	
	User app(user_num);
	
	if (!app.Authentication())
	{
		cout << "Auth Failed!" << endl;
		return 1;
	}
	app.SendSensitiveLog(pureName, message.c_str());

	app.GetFile(string("keywords.txt"));
	app.EndSession();

	EndSSL();

#endif // Session
	cout << "Say something: ";
	//cin >> message;

	return 0;
}


// File End.