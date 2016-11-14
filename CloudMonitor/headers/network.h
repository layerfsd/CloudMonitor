#pragma once
#ifndef _NETWORK_H__
#define _NETWORK_H__

#include "FileMon.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

#include <iostream>
#include <string>


#define RUN_IN_LOCAL		1	// ��������IPΪ��ǰ�������ַ
#define RUN_IN_SCHOOL		0	// ��������IPΪѧУ������ַ
#define RUN_IN_COMPANY		0	// ��������IPΪ����Ĺ�����ַ


#define TEST_FILENAME		"test.docx"
#define MAX_PACKET_SIZE		1024
#define READ_BNR_SIZE		1024*1024
#define MAXBUF				1280
#define SERV_PORT			50005


// ͨ�����������˵�ַ
#if RUN_IN_SCHOOL
#define SERV_ADDR			"10.102.1.116"
#elif RUN_IN_LOCAL
#define SERV_ADDR			"192.168.43.132"
#elif RUN_IN_COMPANY
#define SERV_ADDR			"121.42.146.43"
#endif


#define CONNECT_TIMEOUT		1000

#define SSL_CHANNEL_ON		0
#define SSL_CHANNEL_OFF		1
#define SSL_WORKING			0
#define SSL_NOT_WORKING		1

#define MAX_FILENAME		128
#define MAX_SYSVER			48
#define MAX_USERNAME		32
#define MAC_SIZE			20
#define IP_SIZE				16
#define MAX_RETRY_TINE		1

#define	LOOP_SLEEP_TIME		500 // ���� 500 ����
#define HEART_BEAT_TIME		30  //��������ʱ��(��)

#define CMD_SIZE			3
#define	HEAD_SIZE			7

using namespace std;

struct PacketHeader
{
	char        fileName[MAX_FILENAME];
	char        fileHash[HASH_SIZE];
	char        passwd[MAX_PASSWD];
	char        usrName[MAX_USERNAME];
	char        sysVer[MAX_SYSVER];
	char        mac[MAC_SIZE];
	int         fileSize;
};

struct SSL_Handler 
{
	SSL*     ssl;
	SSL_CTX* ctx;
	SOCKET   sock;
	char     buf[MAXBUF];
};


// ���������������
struct Args
{
	char servIP[32];
	char sendFile[MAX_FILENAME];
	int  servPort;
};

// ͨ�Žṹ��
struct HeadPacket
{
	char	cmd[CMD_SIZE+1];
	char	text[MAXBUF];
};

#define CTL_CONTROL_PLEN  3		// �涨Զ������Ϊ�����ֽڵ��ַ�

typedef bool(*ProcessFunc)(string& logMsg, string& args);


enum ProcessResult
{
	FINISHED = 1,
	FAILED
};
// Զ�̿��ƽṹ��
struct RemoteControl
{
	bool		notExecuted;					// ָ���Ƿ�ִ�й���
	size_t		time;							// �ȴ�������֮��ִ��,Ĭ������ִ��
	char		ctlTxt[CTL_CONTROL_PLEN+1];		// ָ���д
	string		ctlDetails;						// ����
	ProcessFunc func;							// ָ�����
};



/*
ʵ�������˽����� -- ��ʼ
���ڷ���˲��� Python3.5 ������Ϊ������ԣ��ͻ������Ҳ��Python ��д��
Ϊ�ӿ쿪�����࣬�ⲿ�ִ���ֱ�ӽ� Python ����Ϊ���� C++��
	*** Python ԭ�ʹ���ʵ�� ***
�μ�����Ŀ��
	�ͻ��ˣ�tests\PythonCodes\clnt.py
	����ˣ�tests\PythonCodes\serv.py
*/


class User
{
public:

	User(const char *userName);

	~User()
	{
		this->EndSession();
	}

	bool isEndSession();


	// ���շ����Զ�̿���ָ��
	bool GetFromServer();

	// ִ��Զ�̿���ָ��
	bool ExecControl();

	// ����������ͼ���������
	void	ShowCmdDetail()
	{
		cout << "--------------->SERVER BEGIN<--------------------" << endl;
		cout << "[CMD] " << pkt.cmd << endl;
		cout << "[TXT] " << pkt.text << endl;
		cout << "---------------> SERVER END <--------------------\n" << endl;
		return;
	}
	
	// ��ȡ����˷��ؽ��
	bool	GetReplyInfo();
	
	// �����˷���һ����Ϣ��������
	bool	SendInfo(const char *cmdType, const char* text);

	// ͨ�����������͵��ַ������ͻ��˵��û���
	// �����������˫����֤
	bool	Authentication();


	// �����˷��ͱ���ע����Ϣ
	bool	RegisterClient();

	// �ӷ���˻�ȡһ���ļ�
	bool	GetFile(string &sfleName);

	// ������ һ���ļ�
	bool	UploadFile(SFile &file);


	// �����˷���һ����־��Ϣ
	bool	SendLog(const char *fHash, const char *text);

	// ��ȡע����Ϣ
	bool	GetRegistInf();

	// ��ֹ�Ự
	bool	EndSession();

	// Ϊ��֤�����ȶ�,�ͻ��˶�ʱ�����˷���һ��`������`
	bool	HeartBeat();
	
private:
	HeadPacket	pkt;
	char		userName[MAX_USERNAME];
	string		message;
	string		macList;
	string		hdsList;
	string		workDir;
	char		tmpBuf[MAXBUF];
	int			statu;

	// Զ�̿��������б�
	vector<RemoteControl> taskList;
};


// ���̼�ͨ��,һ��ʼ������UDP,ʵ�ʲ��Ժ��ֲ��ȶ�
// ����ʹ�� UDP ͨ�ŷ�ʽ,��Ϊ���� "�����ܵ�"ͨ��.
//int InitUdp(SOCKET& sockSrv);
//void FreeUdp(SOCKET& sockSrv);
int InitTcp();
void FreeTcp();
bool GetInformMessage(char *buf, size_t bufSize);

int InitSSL(char *ip, int port);
int EndSSL();
int IsCnt2Internet();



static const char*  pPipeName = "\\\\.\\pipe\\CloudMonitor";

//���������ܵ�
bool CreateNamedPipeInServer();

// �첽��ȡ�ܵ���Ϣ
bool GetNamedPipeMessage(char* pReadBuf);


DWORD WINAPI ThreadProc(LPVOID lpParam);


//int SSLSend(char *buf, int len);
//int SSLRecv(char *buf, int len);
//int SendFile(char *fileName);
//char *HashFile(char *fileName);
//int argsParse(char ac, char **av, Args *con);
//bool GetReplyInfo(HeadPacket &pkt);

#endif // _HEADER_H__
