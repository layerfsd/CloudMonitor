#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"
#include "process.h"


#include <iostream>

using namespace std;

#pragma comment(lib,"ws2_32.lib")		// ����socket()�׽���
#pragma comment(lib,"libcrypto.lib")	// ssl ���ܺ���
#pragma comment(lib,"libssl.lib")		// ssl ��ȫ�ŵ� 
#pragma comment(lib, "iphlpapi.lib")	// ��ȡ��������״��


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


	// �����½ӿ�,�����Ż�ʱ���ϴ˹���---"��¼���������ļ��Ĺ�ϣ����" ������ļ������ٶ�
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
		"F:\\NutStore\\SSL����\\ClientPython2CPP.txt",
		"F:\\NutStore\\SSL����\\��ȫ�칫��Ϣ���ƽ̨��Ŀ�з�����20160922.docx",
		"F:\\NutStore\\SSL����\\���.doc"
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


	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// ����һ������ TCP �˿�,���������¼�

#if SESSION

	//const char*  user_num = "1234567";
	const char*  user_num = "1237";
	if (NULL != user_name)
	{
		user_num = user_name;
	}

	User app(user_num);
	
	if (!app.Authentication())  // ��֤�˺�	
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}

	char localPath[MAX_PATH];	// ��ʱ�洢�����ļ�·��

	string	netApps;
	while (g_RUNNING)
	{
		//cout << "while looping ..." << endl;
		if (GetInformMessage(localPath, MAX_PATH))
		{
			// �鿴��ǰ�Ƿ����������������
			if (CheckNetworkApps(plst, netApps))
			{
				memset(&file, 0, sizeof(file));
				file.localPath = localPath;

				// �ж��Ƿ�Ϊ�����ļ�
				if (fsFilter(file, kw, hashList, logMessage))
				{
					app.UploadFile(file);
					logMessage += netApps;
					app.SendLog(file.fileHash.c_str(), logMessage.c_str());
				}
			}

		}

		//cout << "No message from >>>Local ..." << endl;
		app.GetFromServer();   // ���շ���˷��͵� Զ�̿���ָ��
		app.ExecControl();    // ����Զ�̿�������
		app.HeartBeat();	  // ���� CLIENT_SLEEP_TIME ���붨ʱ�����˷���һ��������

		if (!g_RUNNING)
		{
			app.EndSession();
			break;
		}
		if (app.isEndSession())  //��������Ƿ񷢳� "��ֹ�Ự"����
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