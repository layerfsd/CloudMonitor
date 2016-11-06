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


#define TEST_FILENAME		"test.docx"
#define MAX_PACKET_SIZE		1024
#define READ_BNR_SIZE		1024*1024
#define MAXBUF				1280
#define SERV_PORT			50005
#define SERV_ADDR			"192.168.43.132"
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
#define MAX_RETRY_TINE		5

#define	LOOP_SLEEP_TIME		500 // 休眠 500 毫秒
#define HEART_BEAT_TIME		30  //定义心跳时间(秒)

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

// 程序输入参数处理
struct Args
{
	char servIP[32];
	char sendFile[MAX_FILENAME];
	int  servPort;
};

// 通信结构体
struct HeadPacket
{
	char	cmd[CMD_SIZE+1];
	char	text[MAXBUF];
};

#define CTL_CONTROL_PLEN  3		// 规定远控命令为三个字节的字符

typedef bool(*ProcessFunc)(string& logMsg, string& args);


enum ProcessResult
{
	FINISHED = 1,
	FAILED
};
// 远程控制结构体
struct RemoteControl
{
	bool		notExecuted;					// 指令是否执行过了
	size_t		time;							// 等待多少秒之后执行,默认立即执行
	char		ctlTxt[CTL_CONTROL_PLEN+1];		// 指令简写
	string		ctlDetails;						// 参数
	ProcessFunc func;							// 指令处理函数
};



/*
实现与服务端交互的 -- 开始
由于服务端采用 Python3.5 开发，为方便测试，客户端起初也是Python 编写。
为加快开发节奏，这部分代码直接将 Python 翻译为代码 C++。
	*** Python 原型代码实现 ***
参见本项目：
	客户端：tests\PythonCodes\clnt.py
	服务端：tests\PythonCodes\serv.py
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


	// 接收服务端远程控制指令
	bool GetFromServer();

	// 执行远程控制指令
	bool ExecControl();

	// 输出命令类型及数据内容
	void	ShowCmdDetail()
	{
		cout << "--------------->SERVER BEGIN<--------------------" << endl;
		cout << "[CMD] " << pkt.cmd << endl;
		cout << "[TXT] " << pkt.text << endl;
		cout << "---------------> SERVER END <--------------------\n" << endl;
		return;
	}
	
	// 获取服务端返回结果
	bool	GetReplyInfo();
	
	// 向服务端发送一条信息或者命令
	bool	SendInfo(const char *cmdType, const char* text);

	// 通过服务器发送的字符串、客户端的用户名
	// 与服务器进行双向认证
	bool	Authentication();


	// 向服务端发送本机注册信息
	bool	RegisterClient();

	// 从服务端获取一个文件
	bool	GetFile(string &sfleName);

	// 向服务端 一个文件
	bool	UploadFile(SFile &file);


	// 向服务端发送一条日志消息
	bool	SendLog(const char *fHash, LogType lt, const char *text);

	// 获取注册信息
	bool	GetRegistInf();

	// 终止会话
	bool	EndSession();

	// 为保证连接稳定,客户端定时向服务端发送一个`心跳包`
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

	// 远程控制任务列表
	vector<RemoteControl> taskList;
};


// 进程间通信,一开始打算用UDP,实际测试后发现不稳定
// 放弃使用 UDP 通信方式,改为以下 "有名管道"通信.
//int InitUdp(SOCKET& sockSrv);
//void FreeUdp(SOCKET& sockSrv);
int InitTcp();
void FreeTcp();
bool GetInformMessage(char *buf, size_t bufSize);

int InitSSL(char *ip, int port);
int EndSSL();
int IsCnt2Internet();



static const char*  pPipeName = "\\\\.\\pipe\\CloudMonitor";

//创建命名管道
bool CreateNamedPipeInServer();

// 异步获取管道消息
bool GetNamedPipeMessage(char* pReadBuf);


DWORD WINAPI ThreadProc(LPVOID lpParam);


//int SSLSend(char *buf, int len);
//int SSLRecv(char *buf, int len);
//int SendFile(char *fileName);
//char *HashFile(char *fileName);
//int argsParse(char ac, char **av, Args *con);
//bool GetReplyInfo(HeadPacket &pkt);

#endif // _HEADER_H__