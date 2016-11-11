#include "network.h"
#include "FileMon.h"
#include "process.h"  // Զ�̿��ƽӿں�������

#include <queue>


static SSL_Handler hdl = { 0 };
static SOCKET GLOBALclntSock;

static HANDLE  hNamedPipe;
static bool    Accept = false;

//  ����һ������,�������� ·���б�
static queue<string> LocalPathList;

// �����������ʱ���ÿ������ʱ��
// ������ͻ��˷���һ��`����` ��ѭ������
static int	   SLEEP_TIMES_PER_HBT = (1000 / LOOP_SLEEP_TIME) * HEART_BEAT_TIME;

namespace session
{
	// ������Ʊ����Ӧ�Ĵ�����
	struct FuncList
	{
		int			ctl;			// ���Ʊ���
		ProcessFunc func;			// ���ƺ���:typedef bool(*ProcessFunc)(string& logMsg, string& args);
		char	    funcDesc[32];   // ���ƺ�������
	};


	const char*		AuthString		= "WHO ARE YOU";
	const char*		CMD_LOG			= "LOG";
	const char*		CMD_ATH			= "ATH";
	const char*		CMD_END			= "END";
	const char*		CMD_DNF			= "DNF";
	const char*		CMD_RPL			= "RPL";
	const char*		CMD_UPD			= "UPD";
	const char*		CMD_BGN			= "BEGIN";
	const char*     CMD_HBT			= "HBT";

	// ����Զ�̿��Ƶİ�ͷΪ `CTL`
	const char*     CMD_CONTROL = "CTL";

	//���������:�ɹ�/ʧ��
	const char*     CTL_RPL_OK		= "COK";
	const char*		CTL_RPL_FAILED  = "CNO";

	const char*		CTL_INVALID_TEXT = "INVALID INSTRUCTIONS";

	const char*     CTL_END_SESSION  = "000";
	const char*     CTL_PROCESS_LIST = "001";
	const char*     CTL_KILL_PROCESS = "002";

	FuncList funcList[] =
	{
		{0, NULL},
		{1, RemoteGetProcessList, "GetProcessList" },
		{2, RemoteKillProcess, "KillProcess" }
	};
	// ����Զ�̿��ƽӿ�����
	const int	    CTL_SUPPORT_NUM = sizeof(funcList) / sizeof(funcList[0]);

	//static RemoteControl ctlPac[] =
	//{
	//	{0, CTL_PROCESS_LIST, NULL}, // get process list
	//	{0, CTL_END_SESSION, NULL}  // end session
	//};



	const char*		FILE_KEEP_DIR	= "DATA\\";
	const char*		INVALID_ACCOUNT = "INVALID";

	const char*		AUTH_FAILED		= "FAILED";
	const char*		AUTH_SUCCESS	= "OK";
	const char*		DNF_SUCCESS		= "OK";


	const int		INVALID_STR_LEN		= strlen(INVALID_ACCOUNT);
	const int		STATUE_CONNECTED	= 1;
	const int		STATUE_DISCONNECTED = 2;

	const int		AUTH_SUCCESS_LEN	= strlen(AUTH_SUCCESS);
	const int		AUTH_FAILED_LEN		= strlen(AUTH_FAILED);
	const int		DNF_SUCCESS_LEN		= strlen(DNF_SUCCESS);

	const int		authStrLen		    = strlen(AuthString);
	const int		CMD_BGN_LEN		    = strlen(CMD_BGN);
	const int		CMD_CONTROL_LEN		= strlen(CMD_CONTROL);

};

using namespace session;

int InitSSL(char *ip, int port)
{

	int ret;
	int cnt;

	SOCKADDR_IN addrSrv;

	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	hdl.ctx = SSL_CTX_new(SSLv23_client_method());

	if (hdl.ctx == NULL)
	{
		ERR_print_errors_fp(stdout);
		exit(1);
	}

	/* start initialize windows socket library */
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(1, 1);
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0) {
		return SSL_CHANNEL_OFF;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return SSL_CHANNEL_OFF;
	}
	/* end initialize windows socket library */

	/* start  establish a traditional TCP connection to server */
	hdl.sock = socket(AF_INET, SOCK_STREAM, 0);
	addrSrv.sin_addr.S_un.S_addr = inet_addr(ip);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(port);
	//    printf("socket created\n");
	//    printf("connecting to %s:%d\n", SERV_ADDR, SERV_PORT);

	cnt = 0;
	while (cnt < MAX_RETRY_TINE) {
		ret = connect(hdl.sock, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		if (ret != 0) {
			cnt += 1;
			printf("Connect Failed\n");
			ret = CONNECT_TIMEOUT * cnt;
			printf("sleeping %d seconds\n", ret / 1000);
			Sleep(ret);
			printf("reconnect to %s:%d %d/%d times...\n", SERV_ADDR, SERV_PORT, cnt, MAX_RETRY_TINE);
		}
		else {
			break;
		}
	}
	if (cnt >= MAX_RETRY_TINE) {
		printf("remote server is not listening on %s:%d\n", SERV_ADDR, SERV_PORT);
		return SSL_CHANNEL_OFF;
	}
	else {
		printf("ssl channel established successfully!\n");
	}
	/* TCP connection established */

	/* start establish a SSL channel upon the previous TCP connection */
	hdl.ssl = SSL_new(hdl.ctx);
	SSL_set_fd(hdl.ssl, hdl.sock);
	if (SSL_connect(hdl.ssl) == -1) {
		ERR_print_errors_fp(stderr);
	}
	/* end create SSL channel */

	return SSL_CHANNEL_ON;
}



int EndSSL()
{
	if (NULL == hdl.ssl) {
		return 0;
	}
	SSL_shutdown(hdl.ssl);
	SSL_free(hdl.ssl);
	closesocket(hdl.sock);
	WSACleanup();   //end socket
	SSL_CTX_free(hdl.ctx);
	memset(&hdl, 0, sizeof(hdl));
	printf("\n=========================================\n\n");
	return 0;
}


int SSLIsWorking()
{
	if (NULL == hdl.ssl) {
		return SSL_NOT_WORKING;
	}
	return SSL_WORKING;
}



//int SendFile(char *fileName)
//{
//	char    buf[MAXBUF];
//	int     ret = 0;
//	size_t     fsize = 0;
//	FILE*   fp = NULL;
//
//	GetFileSize(fileName, &fsize);
//	while (fsize) {
//		printf("resume %d bytes\n", fsize);
//		ret = fread(buf, 1, MAXBUF, fp);
//		fsize -= ret;
//		SSLSend(buf, ret);
//	}
//	printf("resume %d bytes\n", fsize);
//	printf("transform %s done.\n", fileName);
//
//	return 0;
//}


int IsCnt2Internet()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKADDR_IN addrSrv;
	int err;
	struct addrinfo hints;
	struct addrinfo *res, *cur;
	int ret;
	struct sockaddr_in *addr;
	char m_ipaddr[16];
	//char *domainList[] = {
	//	"www.baidu.com",
	//	"www.qq.com",
	//	"www.sina.com"
	//};
	//int cnt = sizeof(domainList) / sizeof(domainList[0]);

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);


	memset(&hints, 0, sizeof(struct addrinfo));
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(80);

	ret = getaddrinfo("www.baidu.com", NULL, &hints, &res);
	if (ret == -1) {
		//perror("getaddrinfo");
		return -1;
	}

	ret = -1;
	for (cur = res; cur != NULL && 0 != ret; cur = cur->ai_next) {
		addr = (struct sockaddr_in *)cur->ai_addr;
		sprintf(m_ipaddr, "%d.%d.%d.%d",
			(*addr).sin_addr.S_un.S_un_b.s_b1,
			(*addr).sin_addr.S_un.S_un_b.s_b2,
			(*addr).sin_addr.S_un.S_un_b.s_b3,
			(*addr).sin_addr.S_un.S_un_b.s_b4);
		//printf("%s\n", m_ipaddr);
		addrSrv.sin_addr.S_un.S_addr = inet_addr(m_ipaddr);
		ret = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
		//std::cout << "ret: " << ret << std::endl;
	}
	
	freeaddrinfo(res);
	closesocket(sockClient);
	WSACleanup();
	return ret;
}


inline int n2hi(int num)
{
	char	ch = 0;
	char*	pos = (char *)&num;

	for (int i = 0; i < 2; ++i)
	{
		ch = pos[i];
		pos[i] = pos[3 - i];
		pos[3 - i] = ch;
	}

	return num;
}


bool User::isEndSession()
{
	return STATUE_DISCONNECTED == statu;
}


bool User::GetFromServer()
{

	static	FD_SET fdRead;
	int		nRet = 0;//��¼���ͻ��߽��ܵ��ֽ���
	static TIMEVAL	tv = { 0, 500 };//���ó�ʱ�ȴ�ʱ��


	if (STATUE_DISCONNECTED == this->statu)
	{
		//cout << "[Error] Disconnected to Server!\n" << endl;
		return false;
	}


	FD_ZERO(&fdRead);
	FD_SET(hdl.sock, &fdRead);



	//ֻ����read�¼����������滹�ǻ��ж�д��Ϣ���͵�
	nRet = select(0, &fdRead, NULL, NULL, &tv);

	if (nRet == 0)
	{//û�����ӻ���û�ж��¼�
		return false;
	}

	//nRet = send(sockfd, buf, nRet, 0);

	if (nRet > 0)
	{
		return this->GetReplyInfo();
	}

	return false;
}

bool User::ExecControl()
{
	bool execStatus = false;
	
	if (this->taskList.size() <= 0)
	{
		//std::cout << "Empty taskList ..." << endl;
		return false;
	}

	// �����������
	for (size_t i = 0; i < this->taskList.size(); i++)
	{
		//if
		if (this->taskList[i].notExecuted && 0 == this->taskList[i].time) // ���������δִ��,�������ָ���Ƿ�Ҫ������ִ��
		{
			/*  ����������ִ�н�����,��������в�ȡ�Ĵ���ʽ:
			*		1. �����������ɾ��������
			*		2. ��ɾ��������,����Ǹ������ѳɹ�ִ��
			*   ���ѡ��ʽ1,���ǵ���ƹ���,���÷�ʽ2
			*   ��������֧�ַ��������Ҷ�Ӧ����������ִ��״��
			*/
			execStatus = this->taskList[i].func(this->message, taskList[i].ctlDetails);		// ����User���е�message���ø�Զ�̴�����,Զ�̴����������Ĵ������洢�� message ��
			if (execStatus)
			{
				this->SendInfo(CTL_RPL_OK, message.c_str());							// ����ɹ�ִ��,֪ͨ����˴����� 
			}
			else
			{
				this->SendInfo(CTL_RPL_FAILED, message.c_str());						// ����ִ��ʧ��,Ҳ֪ͨ����˴����� 
			}
			this->taskList[i].notExecuted = false;								 // ����ִ�к�,�������`δִ��״̬`Ϊ��
			break;
		//end if
		}
	//end for
	}
	return execStatus;
}

User::User(const char *userName)
{
	this->statu = STATUE_DISCONNECTED;

	memset(tmpBuf, 0, sizeof(tmpBuf));
	if (GetCurrentDirectory(MAX_PATH, tmpBuf))  //�õ���ǰ����·��
	{// failed get current dir
		this->workDir = tmpBuf;
		this->workDir += '\\';
	}
	this->workDir += FILE_KEEP_DIR;

	// if not exists workDir
	// try to mkdir
	if (-1 == _access(this->workDir.c_str(), 0))
	{
		cout << "making workDir: " << workDir << endl;
		if (0 != _mkdir(this->workDir.c_str()))
		{
			cout << "mkdir failed!!!!...\n" << endl;
		}
		else
		{
			cout << "mkdir success!!!!...\n" << endl;
		}
	}
	strncpy(this->userName, userName, MAX_USERNAME);
	//cout << "workdir: " << workDir << endl;

	if (0 != InitSSL(SERV_ADDR, SERV_PORT))
	{
		exit(3);
	}

}


bool User::GetReplyInfo()
{
	RemoteControl tmpTask;
	char buf[HEAD_SIZE] = { 0 };
	int	 reveivedSize = 0;
	int  restPktSize = 0;

	if (STATUE_DISCONNECTED == this->statu)
	{
		//cout << "[Error] Disconnected to Server!\n" << endl;
		return false;
	}

	memset(&pkt, 0, sizeof(HeadPacket));

	//printf("receiving %d bytes\n", HEAD_SIZE);
	reveivedSize = SSL_read(hdl.ssl, buf, HEAD_SIZE);
	//printf("reveivedSize: %x\n", reveivedSize);

	memcpy(pkt.cmd, buf, CMD_SIZE);
	memcpy(&restPktSize, buf + CMD_SIZE, 4);
	restPktSize = n2hi(restPktSize);

	if (reveivedSize != HEAD_SIZE)
	{
		std::cout << "Receiving Failed!\n" << endl;
		this->statu = STATUE_DISCONNECTED;
		return false;
	}
	//printf("restPktSize: %d\n", restPktSize);
	SSL_read(hdl.ssl, pkt.text, restPktSize);


	// �ж�ָ������: �Ƿ�Ϊ"Զ�̿���"
	if (!strncmp(pkt.cmd, CMD_CONTROL, CMD_CONTROL_LEN)) // ����յ���һ��Զ�̿���ָ��
	{
		pkt.text[CTL_CONTROL_PLEN] = 0;
		// ָ���ҵ���Ӧ�Ĵ�����
		int instructionCode = atoi(pkt.text);

		// �ж�ָ������Ƿ�֧��
		if ((instructionCode >= 0) && (instructionCode < CTL_SUPPORT_NUM))
		{
			memset(&tmpTask, 0, sizeof(tmpTask));				  // ��ʼ��Զ�̿���ָ��ṹ��	
			strncpy(tmpTask.ctlTxt, pkt.text, CTL_CONTROL_PLEN);  // ��������˷��͵�ָ����
			tmpTask.ctlDetails = pkt.text + CTL_CONTROL_PLEN + 1;	  // ��������˷��͵�ָ���Ӧ��ָ�����
			//cout << "details: [" << tmpTask.ctlDetails << "]" << endl;
			tmpTask.time = 0;									  // Ĭ����������������ִ��,���ȴ�
			tmpTask.notExecuted = true;							  // �趨�����`δִ��`״̬Ϊ��

			if (0 == instructionCode)							// ���������Ҫ��Ͽ�����,��ֱ�Ӵ���
			{
				this->ShowCmdDetail();							// ��������ָ��
				this->statu = STATUE_DISCONNECTED;				// ���õ�ǰ�Ự״̬Ϊ: �Ͽ�����
				return true;
			}
			tmpTask.func = funcList[instructionCode].func;		// Ϊÿһ������ѡ���Ӧ�Ĵ������
			this->taskList.push_back(tmpTask);					// ��Զ�̿���������뱾���������

			cout << "Get new task: {" << endl
				<< "\ttime: " << tmpTask.time << endl
				<< "\tnotExecuted: " << tmpTask.notExecuted << endl
				<< "\tinstructionCode: " << instructionCode << endl
				<< "\tfunc at: " << tmpTask.func << endl
				<< "\tfuncDesc: " << funcList[instructionCode].funcDesc << endl;
			cout << "}" << endl;
			
		}
		else // ���Զ�̿��Ʊ��벻֧��,��֪ͨ����˴���������ϢΪ�Ƿ�ָ��
		{
			std::cout << "!!! " << CTL_INVALID_TEXT << endl;
			this->SendInfo(CMD_RPL, CTL_INVALID_TEXT);
		}

	}

	this->ShowCmdDetail();

	return true;
}


bool User::SendInfo(const char *cmdType, const char* text)
{
	int pktSize = 0;
	int textLen = strlen(text);
	int netLen = n2hi(textLen);

	if (STATUE_DISCONNECTED == this->statu)
	{
		cout << "[Error] Disconnected to Server!\n" << endl;
		return false;
	}



	// ������Ϣͷ
	// ��Ϣͷ��ʽΪ: CMD_TYPE(char[3])+restPktSize(int)
	memcpy(tmpBuf, cmdType, CMD_SIZE);
	
	//printf("[textLen] 0x%x\n", textLen);
	//printf("[netLen]  0x%x\n", netLen);

	memcpy(tmpBuf + CMD_SIZE, &netLen, sizeof(int));

	// ��ʵ����Ϣ���������ͻ���
	memcpy(tmpBuf + HEAD_SIZE, text, textLen);

	// ����������Ϣ���Ĵ�С
	pktSize = HEAD_SIZE + textLen;
	int ret = SSL_write(hdl.ssl, tmpBuf, pktSize);

	cout << "***************>CLIENT BEGIN<********************" << endl;
	cout << "[CMD] " << cmdType << endl;
	cout << "[TXT] " << text << " [textLen] " << textLen << endl;
	cout << "***************> CLIENT END <********************\n" << endl;

	return true;
}

// ���������Ƿ�Ϸ�
bool RecognizeServer(const char* servWords)
{
	return !memcmp(servWords, AuthString, authStrLen);
}


inline bool IsLoginOK(const char* buf)
{
	return !memcmp(AUTH_SUCCESS, buf, AUTH_SUCCESS_LEN);
}


inline bool IsRegistOK(const char* buf)
{
	return !memcmp(AUTH_SUCCESS, buf, AUTH_SUCCESS_LEN);
}



string _get_mac()
{
	return string("MAC:2 11:22:33:44:55:66-wireless aa:bb:cc:dd:ee:ff-wired\n");
}

string _get_hds()
{
	return string("HDS:2 9CD29CD2-machine C5F3BFE9-ssd\n");
}


bool User::RegisterClient()
{
	this->macList = _get_mac();
	this->hdsList = _get_hds();

	this->message = macList + hdsList;
	cout << "Computer Information: " << endl;
	cout << message;

	this->SendInfo(CMD_ATH, message.c_str());
	this->GetReplyInfo();

	return IsRegistOK(this->pkt.text);
}

inline bool IsInvalidUser(const char *status)
{
	return !memcmp(status, INVALID_ACCOUNT, INVALID_STR_LEN);
}

bool User::Authentication()
{
	this->statu = STATUE_CONNECTED;

	GetReplyInfo();
	if (!RecognizeServer(pkt.text))
	{
		cout << "Invalid Server!" << endl;
		return false;
	}
	
	// ���Ե�½
	SendInfo(CMD_ATH, userName);
	GetReplyInfo();

	if (IsInvalidUser(pkt.text))
	{
		cout << "Invalid user: " << this->userName;
		return false;
	}

	if (IsLoginOK(pkt.text) || RegisterClient())
	{
		cout << "Client Login success." << endl;
		return true;
	}

	cout << "Register client failed!" << endl;
	return false;
}


bool User::SendLog(const char* fHash, const char* text)
{
	time_t t = time(0);   // get time now
	struct tm* now = localtime(&t);
	char   tmp[MAX_LOG_SIZE];

	// ��ȡʱ��
	// ��ʱ����ļ���ϣƴ��ͬһ�黺��
	sprintf(tmp, "%d-%02d-%02d %02d:%02d\n%s\n",
		now->tm_year + 1900,
		now->tm_mon + 1,
		now->tm_mday,
		now->tm_hour,
		now->tm_min,
		fHash);


	// ������־
	this->message = tmp;	
	this->message += text;	// ׷�� (�ؼ�����Ϣ+��־����)

	return this->SendInfo(CMD_LOG, this->message.c_str());
}


bool User::GetRegistInf()
{
	return false;
}

bool User::EndSession()
{
	cout << "Disconnecting from Server...\n" << endl;
	if (this->isEndSession())
	{
		cout << "Before I say Good-Bye to Server" << endl;
		cout << "Server Already Disconnected from me ...\n" << endl;
		EndSSL();
		return true;
	}
	this->SendInfo(CMD_END, "Bye-bye.");
	this->statu = STATUE_DISCONNECTED;
	EndSSL();

	return true;
}


inline bool IsServerExistFile(const char *buf)
{
	return !memcmp(buf, DNF_SUCCESS, DNF_SUCCESS_LEN);
}


inline bool IsHashEqual(const char* fileName, char* hash)
{
	char	localFileHash[HASH_SIZE];
	if (!HashFile(fileName, localFileHash))
	{
		return false;
	}
	return !memcmp(localFileHash, hash, HASH_SIZE);
}


// ������˴���������ļ�ʱ,Ϊ�˷�ֹ��ȡ���ظ��ļ�
// �����������ŵ��ļ��Ƿ��뱾�ص��ļ���ϣֵ��ͬ
// �����ͬ,��˵�����ز�������ͬ�ļ�,��������
// �����ͬ,��˵�������Ѿ�������ͬ�ļ�,֪ͨ����˱����Ѵ��������ļ�
// ˫������ļ����󽻻�
/*
	Ϊ�������,���Ըı�ͻ��������˷����ļ�����ʱ,
	˫�����½�һ����(��)�̵�������.

	Ϊ�������ȿ���,��ʱ��������̴���
*/
bool User::GetFile(string &FileName)
{
	this->SendInfo(CMD_DNF, FileName.c_str());
	
	// Get file state {exist or not}
	GetReplyInfo();
	if (!IsServerExistFile(this->pkt.text))
	{
		GetReplyInfo();
		cout << "Server Does not have: " << FileName << endl;
		return false;
	}
	
	// get fileHash
	GetReplyInfo();
	if (IsHashEqual(FileName.c_str(), this->pkt.text))
	{
		this->SendInfo(CMD_RPL, "EXISTED");
		return true;
	}
	else
	{
		this->SendInfo(CMD_RPL, "BEGIN");
	}
	
	GetReplyInfo();
	int	fileSize = atoi(pkt.text);
	cout << FileName << " size: " << pkt.text << endl;
	cout << "atoi: " << fileSize << endl;

	string saveFilePath = this->workDir + FileName;

	if (fileSize <= 0)
	{
		return false;
	}
	FILE *fp = NULL;

	if ((fp = fopen(saveFilePath.c_str(), "wb")) == NULL)
	{
		return false;
	}

	int restSize = fileSize;
	int recvSize = 0;

	while (restSize > 0)
	{
		memset(tmpBuf, 0, sizeof(tmpBuf));
		recvSize = SSL_read(hdl.ssl, tmpBuf, MAXBUF);
		fwrite(tmpBuf, 1, recvSize, fp);
		restSize -= recvSize;
	}
	cout << "Get [" << saveFilePath << "] Done!" << endl;
	fclose(fp);
	return true;
}

inline bool IsServerWantThis(const char *rpl)
{
	return !memcmp(rpl, CMD_BGN, CMD_BGN_LEN);
}

bool User::UploadFile(SFile &file)
{
	cout << "Sending " << file.fileName << "to server..." << endl;
	this->SendInfo(CMD_UPD, file.fileName.c_str());
	this->SendInfo(CMD_RPL, file.fileHash.c_str());
	
	this->GetReplyInfo();
	if (!IsServerWantThis(pkt.text))
	{
		// ���������Ѿ����ڵ�ǰ�ļ���
		return true;
	}
	// �������뷢��
	char fsize[256];
	sprintf(fsize, "%d %s", file.encSize, file.encPasswd.c_str());
	this->SendInfo(CMD_RPL, fsize);

	FILE *fp = NULL;
	if ((fp = fopen(file.encPath.c_str(), "rb")) == NULL)
	{
		return false;
	}
	else
	{
		cout << "open " << file.encPath << "OK" << endl;
	}

	int restSize = file.encSize;
	int readSize = 0;
	// ����,���㷢�ʹ���
	int cnt = 0;
	while (restSize > 0)
	{
		//cout << "Remaining " << restSize << " bytes to transform..." << endl;
		memset(tmpBuf, 0, sizeof(tmpBuf));
		readSize = fread(tmpBuf, 1, MAXBUF, fp);
		//cout << "sending " << readSize << "bytes " << endl;
		// ������,������
		//SSL_write(hdl.ssl, tmpBuf, MAXBUF);
		SSL_write(hdl.ssl, tmpBuf, readSize);
		cnt += 1;
		restSize -= readSize;
	}
	cout << "sent " << cnt << " times." << endl;
	cout << "Sent [" << file.encPath << "] Done!" << endl;
	fclose(fp);

	return true;
}


bool User::HeartBeat()
{
	static int count = 0;   // ���һ����̬��������������������

	if (count >= SLEEP_TIMES_PER_HBT)
	{
		count = 0;
		return this->SendInfo(CMD_HBT, CMD_HBT);
	}

	//cout << "LOOP_SLEEP_TIME: " << count << endl;

	count += 1;
	Sleep(LOOP_SLEEP_TIME);

	return true;
}



bool CreateNamedPipeInServer()
{
	HANDLE                    hEvent;
	OVERLAPPED                ovlpd;
	DWORD					  cnt = 1;
	//������Ҫ���������ܵ�
	//���ﴴ������˫��ģʽ��ʹ���ص�ģʽ�������ܵ�
	hNamedPipe = CreateNamedPipe(pPipeName,
		PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
		0, cnt, 1024, 1024, 0, NULL);
	if (INVALID_HANDLE_VALUE == hNamedPipe)
	{
		hNamedPipe = NULL;
		cout << "���������ܵ�ʧ�� ..." << endl << endl;
		return false;
	}
	//����¼��Եȴ��ͻ������������ܵ�
	//���¼�Ϊ�ֶ������¼����ҳ�ʼ��״̬Ϊ���ź�״̬

#if SHIT
	 nt = 10;
	while (cnt--)
	{
#endif
		hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!hEvent)
		{
			cout << "�����¼�ʧ�� ..." << endl << endl;
			return false;
		}
		memset(&ovlpd, 0, sizeof(OVERLAPPED));
		//���ֶ������¼����ݸ� ovlap ����
		ovlpd.hEvent = hEvent;


		//�ȴ��ͻ�������
		if (!ConnectNamedPipe(hNamedPipe, &ovlpd))
		{
			if (ERROR_IO_PENDING != GetLastError())
			{
				CloseHandle(hNamedPipe);
				CloseHandle(hEvent);
				cout << "�ȴ��ͻ�������ʧ�� ..." << endl << endl;
				return false;
			}
		}
		//�ȴ��¼� hEvent ʧ��
		if (WAIT_FAILED == WaitForSingleObject(hEvent, INFINITE))
		{
			CloseHandle(hNamedPipe);
			CloseHandle(hEvent);
			cout << "�ȴ�����ʧ�� ..." << endl << endl;
			return false;
		}
		CloseHandle(hEvent);
#if SHIT
	}
#endif

	return true;
}


bool GetNamedPipeMessage(char* pReadBuf)
{
	DWORD   TotalBytesAvail = 0;
	bool    ret = false;

	// ����Ƿ��Ѿ����ܵ�����
	if (!Accept)
	{
		return false;
	}

	memset(pReadBuf, 0, MAX_PATH);
	PeekNamedPipe(hNamedPipe, pReadBuf, MAX_PATH, NULL, &TotalBytesAvail, NULL);
	if (TotalBytesAvail > 0)
	{
		//�������ܵ��ж�ȡ����
		if (!ReadFile(hNamedPipe, pReadBuf, MAX_PATH, NULL, NULL))
		{
			cout << "��ȡ����ʧ�� ..." << endl << endl;
			ret = false;
		}
		else
		{
			ret = true;
		}
	}

	return ret;
}


int InitTcp()
{
	SOCKET serversoc;
	SOCKADDR_IN serveraddr;
	SOCKADDR_IN clientaddr;
	int len;

	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);	//initial Ws2_32.dll by a process
	if ((serversoc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)	//create a tcp socket
	{
		printf("Create socket fail!\n");
		return -1;
	}

	//	int iResult;
	//	u_long iMode = 0;
	//	iResult = ioctlsocket(serversoc, FIONBIO, &iMode);
	//	if (iResult != NO_ERROR)
	//	{
	//		printf("ioctlsocket failed with error: %d\n", iResult);
	//	}


	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(50006);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(serversoc, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) != 0)
	{
		printf("Bind fail!\n");
		return -1;
	}

	//start listen, maximum length of the queue of pending connections is 1
	printf("Start listen...\n");
	if (listen(serversoc, 1) != 0)
	{
		printf("Listen fail!\n");
		return -1;
	}

	len = sizeof(SOCKADDR_IN);
	char tPath[MAX_PATH];

RESTART_LISTEN:
	while (1)
	{
		//waiting for connecting
		if ((GLOBALclntSock = accept(serversoc, (SOCKADDR *)&clientaddr, &len)) <= 0)
		{
			printf("Accept fail!\n");
			goto RESTART_LISTEN;
		}

		printf("Get Hook Connected\n");
		Accept = true;
		while (Accept)
		{
			int ret = 0;

			memset(tPath, 0, sizeof(tPath));
			ret = recv(GLOBALclntSock, tPath, MAXBUF, 0);
			printf("[CLNT-RECV:%d] %s\n", ret, tPath);

			if (3 == ret && !memcmp("BYE", tPath, 3))
			{
				printf("Hook Service Quit.\n");
				goto _END_RECV;
			}

			if (ret > 0)		//�����ɹ�����,�Ű���Ϣ���뻺�����
			{
				int sent = send(GLOBALclntSock, tPath, ret, 0);
				//cout << "sent: " << sent << "bytes" << endl;
				string tmp = tPath;
				LocalPathList.push(tPath);
			}
			else if (0 == ret)
			{
				continue;
			}
			else
			{
				Accept = false;
				closesocket(GLOBALclntSock);
				goto RESTART_LISTEN;
			}

		}// inner while(Accept)
	}// outter while(1)
_END_RECV:
	return 0;

}

void FreeTcp()
{
	return;
}


bool GetInformMessage(char *buf, size_t bufSize)
{
	if (LocalPathList.size() > 0)
	{
		strncpy(buf, LocalPathList.front().c_str(), bufSize);
		LocalPathList.pop();
		return true;
	}

	return false;
}




DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	//cout << "sub thread started\n" << endl;
	//CreateNamedPipeInServer();
	InitTcp();
	return 0;
}