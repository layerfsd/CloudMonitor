#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"
#include "process.h"

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


	// �����½ӿ�,�����Ż�ʱ���ϴ˹���---"��¼���������ļ��Ĺ�ϣ����" ������ļ������ٶ�
	//LoadHashList(hashPath, hashList);


	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// ����һ������ TCP �˿�,���������¼�


	const char*  user_num = "1234567";
	char  authBuf[64];
	memset(authBuf, 0, sizeof(authBuf));

	// �����û��������ʽ,�Իس����ָ�
	sprintf(authBuf, "%s\n%s", user_name, user_pass);
	//const char*  user_num = "1234568";
	if (NULL != user_name)
	{
		user_num = user_name;
	}

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