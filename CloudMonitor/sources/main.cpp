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

#pragma comment(lib,"ws2_32.lib")		// ����socket()�׽���
#pragma comment(lib,"libcrypto.lib")	// ssl ���ܺ���
#pragma comment(lib,"libssl.lib")		// ssl ��ȫ�ŵ� 
#pragma comment(lib, "iphlpapi.lib")	// ��ȡ��������״��


#define CONTROL				0
#define FULL_DEBUG			0
#define DEBUG_PARSE_FILE	0
#define SESSION				1


void InitDir();					//patches.cpp

BOOL g_RUNNING = TRUE;


// ɾ����ʱ�ļ�
void CleanTmpFiles(SFile& file)
{
	remove(file.savedPath.c_str());		// tmp\doc
	remove(file.encPath.c_str());		// tmp\aes
	remove(file.txtPath.c_str());		// tmp\txt
}

// ��ȡ��������������ַ
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

	// ���ó����Զ�����ʱ��Ĭ�ϴ�ע����н�������֤��Ϣ
	if (2 == argc && !strncmp(argv[1], "--autostart", 32))
	{
		if (!GetAuth(&act))
		{
			printf("[FAILED] GetAuth\n");
			exit(1);
		}
	}
	// �û��ֶ����иó�����ˢ����֤��Ϣ��ע���
	if (argc == 3)
	{
		// ��ʼ���˻���Ϣ
		strcpy_s(act.username, 32, argv[1]);
		strcpy_s(act.password, 32, argv[2]);
		SetAuth(&act);
	}

	InitDir();


	// �����½ӿ�,�����Ż�ʱ���ϴ˹���---"��¼���������ļ��Ĺ�ϣ����" ������ļ������ٶ�
	//LoadHashList(hashPath, hashList);


	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// ����һ������ TCP �˿�,���������¼�


	char  authBuf[128];
	memset(authBuf, 0, sizeof(authBuf));

	string wiredMac;

	if (!GetWiredMac(wiredMac))
	{
		cout << "GetWiredMac Error" << endl;
		exit(1);
	}
	// �����û��������ʽ,�Իس����ָ�
	sprintf(authBuf, "%s\n%s\n%s", act.username, act.password, wiredMac.c_str());
	//const char*  user_num = "1234568";

	char localPath[MAX_PATH];	// ��ʱ�洢�����ļ�·��

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

			// �ж��Ƿ�Ϊ�����ļ�
			if (fsFilter(file, kw, hashList, logMessage))
			{
				cout << "Logmsg: " << logMessage << endl;
			}
			CleanTmpFiles(file);
		}
	}
#else
	User app(authBuf);

	if (!app.Authentication())  // ��֤�˺�	
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}

	// ÿ���������ȸ��¹ؼ����б�
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

			// �ж��Ƿ�Ϊ�����ļ�
			if (fsFilter(file, kw, hashList, logMessage))
			{
				app.UploadFile(file);
				cout << "Logmsg: " << logMessage << endl;
				app.SendLog(file.fileHash.c_str(), logMessage.c_str());
			}
			CleanTmpFiles(file);
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
	}
#endif

	return 0;
}


// File End.