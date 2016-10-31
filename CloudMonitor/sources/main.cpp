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
#define SESSION				1


inline void InitDir()
{

	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //�õ���ǰģ��·��
	strcat(strModule, "//..//");     //����Ϊ��ǰ����·��Ϊ��ʱ����һ��
	SetCurrentDirectory(strModule);
	GetCurrentDirectory(sizeof(strModule), strModule);

	cout << "Working Dir: " << strModule << endl;
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
	vector<Process> plst;

	SFile file;
	char localPath[MAX_PATH];	// ��ʱ�洢�����ļ�·��

	InitDir();
	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}


	// �����½ӿ�,�����Ż�ʱ���ϴ˹���---"��¼���������ļ��Ĺ�ϣ����" ������ļ������ٶ�
	//LoadHashList(hashPath, hashList);


#if CONTROL
	string	message;
	if (!GetProcessList(plst))
	{
		cout << "GetProcessList: empty!" << endl;
		return -1;
	}
	//plst[0].shutdown = true;
	if (KillProcess(plst))
	{
		GenKillResult(plst, message);
		cout << "kill result: [" << message << "]" << endl;
	}
	else
	{
		cout << "No Process to kill ..." << endl;
	}
	ShowProcessList(plst);
#endif

#if DEBUG_PARSE_FILE
	file.localPath = "F:\\NutStore\\SSL����\\ClientPython2CPP.txt";
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


	char*  user_num = "1234567";
	
	User app(user_num);
	
	if (!app.Authentication())  // ��֤�˺�	
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}

	CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// ����һ������ TCP �˿�,���������¼�

	while (true)
	{
		//cout << "while looping ..." << endl;
		if (GetInformMessage(localPath, MAX_PATH))
		{
			memset(&file, 0, sizeof(file));
			file.localPath = localPath;
			if (fsFilter(file, kw, hashList, logMessage))
			{
				cout << "logMessag: " << logMessage << endl;
				app.UploadFile(file);
				app.SendLog(file.fileName.c_str(), FILE_NETEDIT, logMessage.c_str());
			}
		}

		//cout << "No message from >>>Local ..." << endl;
		app.GetFromServer();   // ���շ���˷��͵� Զ�̿���ָ��
		app.ProcessControl();  // ����Զ�̿�������
		app.HeartBeat();	   // ���� CLIENT_SLEEP_TIME ���붨ʱ�����˷���һ��������

		if (app.isEndSession())  //��������Ƿ񷢳� "��ֹ�Ự"����
		{
			break;
		}
	}

#endif // Session

	return 0;
}


// File End.