#include <string.h>
#include <iostream>
#include <stdio.h>

#include "network.h"
#include "Encrypt.h"
#include "NetMon.h"
#include "parsedoc.h"
#include "FileMon.h"
#include "process.h"

#include "LocalTCPServer.h"
#include "AutoStart.h"
#include "patches.h"
#include "PickFiles.h"
#include "mUSB.h"

using namespace std;

#pragma comment(lib, "ws2_32.lib")		// ����socket()�׽���
#pragma comment(lib, "libcrypto.lib")	// ssl ���ܺ���
#pragma comment(lib, "libssl.lib")		// ssl ��ȫ�ŵ� 
#pragma comment(lib, "iphlpapi.lib")	// ��ȡ��������״��

#define LOCAL_SCAN

#define CONTROL				0
#define FULL_DEBUG			0
#define DEBUG_PARSE_FILE	0
#define SESSION				1

// ���Ʊ�������ѭ��
BOOL g_RUNNING = TRUE;


// ��ȡ��������������ַ
bool GetWiredMac(string& wiredMac);


// ��ȡ����ɨ��������
vector<HashItem> hashList;

// �ؼ����б�����
vector<Keyword> kw;

static string keywordPath = KEYWORD_PATH;
static string hashPath = HASHLST_PATH;

int main(int argc, char *argv[])
{

	// ��¼��ǰ�������������
	vector<Connection> cons;

	// ��ʱ��¼��־
	string logMessage;
	// ��ʱ�ļ���Ϣ
	SFile file;
	
	// �û���¼�˺�
	Account act;

	// �����ն˵���������:���Բ����أ���ʽ����ʱ������
	// ͨ���ж���������ʶ�������ǰ���ڵ�״̬:(����|��ʽ����)
	bool hide = false;

	SetWorkPath();

	// ���ʲô����Ҳû�У����˳�������
	if (1 == argc)
	{
		printf("Need args\n");
		return 1;
	}
	if (argc > 3)
	{
		printf("invalid args\n");
		return 1;
	}
	// ���ó����Զ�����ʱ��Ĭ�ϴ�ע����н�������֤��Ϣ
	if (2 == argc)
	{
		if (!strncmp(argv[1], "--autostart", 32))
		{
			hide = true;
		}
		else if (!strncmp(argv[1], "--start", 32))	// ��ע����ж�ȡ��֤��Ϣ
		{
			hide = false;
		}
		else
		{
			printf("Unknown Args: [%s]\n", argv[1]);
			exit(1);
		}
		if (!GetAuth(&act))
		{
			printf("[FAILED] GetAuth\n");
			return 1;
		}
	}

	// �õ����������������û���������
	if (argc == 3)
	{
		// ��ʼ���˻���Ϣ
		strncpy(act.username, argv[1], 32);
		strncpy(act.password, argv[2], 32);
	}

	string wiredMac;			// ��ʱ��ȡ������ַ
	char authBuf[128];			// ��¼��֤��Ϣ

	if (!GetWiredMac(wiredMac))
	{
		cout << "GetWiredMac Error" << endl;
		return 1;
	}

	memset(authBuf, 0, sizeof(authBuf));

	// �����û��������ʽ,�Իس����ָ�
	sprintf(authBuf, "%s\n%s\n%s", act.username, act.password, wiredMac.c_str());

	InitDir(hide);

	User app(authBuf);
	USB	 usb;

	if (!app.Authentication())  // ��֤�˺�	
	{
		cout << "Auth Failed!" << endl;
		return -1;
	}

	if (argc == 3)
	{
		// ��ʼ���˻���Ϣ
		SetAuth(&act);
	}

	// ��������֤�ɹ�����������ؼ�������, IO��ط���
	HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);		// ����һ������ TCP �˿�,��IO��������ͨ��


	// �����½ӿ�,�����Ż�ʱ���ϴ˹���---"��¼���������ļ��Ĺ�ϣ����" ������ļ������ٶ�
	//LoadHashList(hashPath, hashList);


	// �û��ֶ����иó����ҵ�¼�ɹ�����ˢ����֤��Ϣ��ע���
	if (3 == argc)
	{
		SetAuth(&act);
	}
	// ÿ���������ȸ��¹ؼ����б�
	app.GetFile(string(KEYWORDS));
	if (!LoadKeywords(keywordPath, kw))
	{
		cout << "[Error]: " << "Loading keywords Failed!!!\n" << endl;
		return -1;
	}

	char localPath[MAX_PATH];	// ��ʱ�洢�����ļ�·��

	while (g_RUNNING)
	{
		// ���USB �ӿ�
		if (CheckUsbDevice(usb))
		{
			logMessage = usb.getMessage();
			// cout << logMessage;
			logMessage = GBKToUTF8(usb.getMessage().c_str());
			app.SendLog(NULL, logMessage.c_str(), USB_PLUG_EVENT);
		}

		// �ӻ�������ȡ���ļ����¼�֪ͨ��
		if (GetInformMessage(localPath, MAX_PATH))
		{
			printf("get new task: %s\n", localPath);
			memset(&file, 0, sizeof(file));
			file.localPath = localPath;

			// �ж��Ƿ�Ϊ�����ļ�
			if (fsFilter(file, kw, hashList, logMessage))
			{
				wrapEncreytFile(file);		// ���������ļ�
				app.UploadFile(file);		// �ϴ����ܺ�������ļ�
				cout << "Logmsg: " << logMessage << endl;	// ��ӡ�ļ��ؼ���ƥ������
				app.SendLog(file.fileHash.c_str(), logMessage.c_str());		// �ϴ���־
			}
			CleanTmpFiles(file);
		}

		app.GetFromServer();   // ���շ���˷��͵� Զ�̿���ָ��

		app.ExecControl();    // ����Զ�̿�������
		app.HeartBeat();	  // ���� CLIENT_SLEEP_TIME ���붨ʱ�����˷���һ��������

		if (!g_RUNNING)
		{
			app.EndSession();
			break;
		}		
	}
	// �ȴ��߳�ִ�н���
	// WaitForSingleObject(hThread, INFINITE);

	return 0;
}


// File End.
