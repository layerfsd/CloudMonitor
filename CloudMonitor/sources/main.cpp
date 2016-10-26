#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"

#include <iostream>

using namespace std;

#pragma comment(lib,"ws2_32.lib")		// ����socket()�׽���
#pragma comment(lib,"libcrypto.lib")	// ssl ���ܺ���
#pragma comment(lib,"libssl.lib")		// ssl ��ȫ�ŵ�
#pragma comment(lib, "iphlpapi.lib")	// ��ȡ��������״��

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


	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}
	// �����½ӿ�,�����Ż�ʱ���ϴ˹���---���������ļ��Ĺ�ϣ����������ļ������ٶ�
	//LoadHashList(hashPath, hashList);

#if SESSION


	char*  user_num = "3130931002";

	InitSSL(SERV_ADDR, SERV_PORT);
	
	User app(user_num);
	
	if (!app.Authentication())
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}

	cout << "CreateNamedPipeInServer..." << endl;
	CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
	//cout << "CreateNamedPipeInServer Success" << endl;

	char lpath[MAX_PATH];
	while (true)
	{
		cout << "while looping ..." << endl;
		if (!GetNamedPipeMessage(lpath))
		{
			cout << "No message from NamePipe..." << endl;
		}
		else
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

		Sleep(5000);
	}
	//app.GetFile(string("keywords.txt"));
	app.EndSession();

	EndSSL();

#endif // Session

	return 0;
}


// File End.