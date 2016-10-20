#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"

#include <iostream>

using namespace std;

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"libcrypto.lib")	
#pragma comment(lib,"libssl.lib")
#pragma comment(lib, "iphlpapi.lib")

#define FULL_DEBUG 0

int main(int argc, char *argv[])
{
#if FULL_DEBUG
	char mac[32];
	vector<Connection> cons;
	vector<Service> KeyPorts;

	//InitSSL(SERV_ADDR, 50005);
	EndSSL();
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
	char genName[] = "safe.txt";
	char keywordFile[] = "res\\keywords.txt";

	ReadKeywords(keywordFile, kw);
	ParseFile2Text(docName, genName);
	KeywordFilter(kw, genName, message);
	cout << message << endl;
#endif

	return 0;
}
// File End.