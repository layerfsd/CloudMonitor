#include <string.h>
#include <iostream>

#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"
#include "process.h"
#include "../AutoStart.h"
#include "../patches.h"
#include "../PickFiles.h"

using namespace std;

#pragma comment(lib,"ws2_32.lib")		// ����socket()�׽���
#pragma comment(lib,"libcrypto.lib")	// ssl ���ܺ���
#pragma comment(lib,"libssl.lib")		// ssl ��ȫ�ŵ� 
#pragma comment(lib, "iphlpapi.lib")	// ��ȡ��������״��

#define LOCAL_SCAN

#define CONTROL				0
#define FULL_DEBUG			0
#define DEBUG_PARSE_FILE	0
#define SESSION				1


// ���Ʊ�������ѭ��
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
	string logMessage;

	vector<Keyword> kw;
	vector<Connection> cons;
	vector<Service> KeyPorts;
	vector<HashItem> hashList;
	vector<Process> plst;

	// ���汾��Ӳ�̵����з��Ϻ�׺���ļ�
	vector<string> collector;
	vector<string> uploadList;

	SFile file;
	Account act;
	bool hide = false;


	if (1 == argc)
	{
		printf("Need args\n");
		return 1;
	}

	// ���ó����Զ�����ʱ��Ĭ�ϴ�ע����н�������֤��Ϣ
	if (2 == argc && !strncmp(argv[1], "--autostart", 32))
	{
		hide = true;
		if (!GetAuth(&act))
		{
			printf("[FAILED] GetAuth\n");
			return 1;
		}
	}

	if (argc == 3)
	{
		// ��ʼ���˻���Ϣ
		strcpy_s(act.username, 32, argv[1]);
		strcpy_s(act.password, 32, argv[2]);
	}

	InitDir(hide);


	// �����½ӿ�,�����Ż�ʱ���ϴ˹���---"��¼���������ļ��Ĺ�ϣ����" ������ļ������ٶ�
	//LoadHashList(hashPath, hashList);

	string wiredMac;

	if (!GetWiredMac(wiredMac))
	{
		//cout << "GetWiredMac Error" << endl;
		return 1;
	}

	char  authBuf[128];
	memset(authBuf, 0, sizeof(authBuf));

	// �����û��������ʽ,�Իس����ָ�
	sprintf(authBuf, "%s\n%s\n%s", act.username, act.password, wiredMac.c_str());

	char localPath[MAX_PATH];	// ��ʱ�洢�����ļ�·��
	string keywords = "keywords.txt";
	

	User app(authBuf);

	if (!app.Authentication())  // ��֤�˺�	
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}

	// �û��ֶ����иó����ҵ�¼�ɹ�����ˢ����֤��Ϣ��ע���
	if (3 == argc)
	{
		SetAuth(&act);
	}
	// ÿ���������ȸ��¹ؼ����б�
	app.GetFile(keywords);
	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}


#ifdef LOCAL_SCAN
	PickLocalPath(collector);
	cout << "ɨ�赽�ļ���������" << collector.size() << endl;

	for (size_t i = 0; i < collector.size(); i++)
	{
		file.localPath = collector[i];
		// �ж��Ƿ�Ϊ�����ļ�
		if (fsFilter(file, kw, hashList, logMessage))
		{
			cout << file.utf8Path << endl;
			cout << logMessage << endl;
			uploadList.push_back(file.localPath);
		}
	}

	cout << "���ϴ��ļ�������" << uploadList.size() << endl;

	return 0;
#endif




	while (g_RUNNING)
	{
		if (GetInformMessage(localPath, MAX_PATH))
		{
			memset(&file, 0, sizeof(file));
			file.localPath = localPath;

			// �ж��Ƿ�Ϊ�����ļ�
			if (fsFilter(file, kw, hashList, logMessage))
			{
				wrapEncreytFile(file);
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

	return 0;
}


// File End.