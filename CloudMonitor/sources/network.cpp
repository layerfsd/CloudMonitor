#include "network.h"
#include "FileMon.h"

static SSL_Handler hdl = { 0 };

namespace session
{
	const char*		AuthString		= "WHO ARE YOU";
	const char*		CMD_LOG			= "LOG";
	const char*		CMD_ATH			= "ATH";
	const char*		CMD_END			= "END";
	const char*		CMD_DNF			= "DNF";
	const char*		CMD_RPL			= "RPL";
	const char*		CMD_UPD			= "UPD";

	const char*		FILE_KEEP_DIR	= "DATA\\";

	const int		STATUE_CONNECTED	= 1;
	const int		STATUE_DISCONNECTED = 2;

	const char*		AUTH_FAILED		= "FAILED";
	const char*		AUTH_SUCCESS	= "OK";
	const char*		DNF_SUCCESS		= "OK";

	const int		AUTH_SUCCESS_LEN = strlen(AUTH_SUCCESS);
	const int		AUTH_FAILED_LEN  = strlen(AUTH_FAILED);
	const int		DNF_SUCCESS_LEN = strlen(DNF_SUCCESS);

	const int		authStrLen		 = strlen(AuthString);
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


User::User(const char *userName)
{
	this->statu = STATUE_DISCONNECTED;

	memset(tmpBuf, 0, sizeof(tmpBuf));
	if (GetCurrentDirectory(MAX_PATH, tmpBuf))  //得到当前工作路径
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
	cout << "workdir: " << workDir << endl;
}


bool User::GetReplyInfo()
{
	char buf[HEAD_SIZE] = { 0 };
	int	 reveivedSize = 0;
	int  restPktSize = 0;

	if (STATUE_DISCONNECTED == this->statu)
	{
		cout << "[Error] Disconnected to Server!\n" << endl;
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
		cout << "Receiving Failed!\n" << endl;
		return false;
	}
	//printf("restPktSize: %d\n", restPktSize);
	SSL_read(hdl.ssl, pkt.text, restPktSize);


	ShowCmdDetail();

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



	// 构造消息头
	// 消息头格式为: CMD_TYPE(char[3])+restPktSize(int)
	memcpy(tmpBuf, cmdType, CMD_SIZE);
	
	//printf("[textLen] 0x%x\n", textLen);
	//printf("[netLen]  0x%x\n", netLen);

	memcpy(tmpBuf + CMD_SIZE, &netLen, sizeof(int));

	// 将实体消息拷贝进发送缓存
	memcpy(tmpBuf + HEAD_SIZE, text, textLen);

	// 计算整个消息包的大小
	pktSize = HEAD_SIZE + textLen;
	int ret = SSL_write(hdl.ssl, tmpBuf, pktSize);

	cout << "***************>CLIENT BEGIN<********************" << endl;
	cout << "[CMD] " << cmdType << endl;
	cout << "[TXT] " << text << " [textLen] " << textLen << endl;
	cout << "***************> CLIENT END <********************\n" << endl;

	return true;
}

// 检查服务器是否合法
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


bool User::Authentication()
{
	this->statu = STATUE_CONNECTED;

	GetReplyInfo();
	if (!RecognizeServer(pkt.text))
	{
		cout << "Invalid Server!" << endl;
		return false;
	}
	
	// 尝试登陆
	SendInfo(CMD_ATH, userName);
	GetReplyInfo();

	if (IsLoginOK(pkt.text) || RegisterClient())
	{
		cout << "Client Login success." << endl;
		return true;
	}

	cout << "Register client failed!" << endl;
	return false;
}


bool User::SendLog(const char* pureName, LogType lt, const char* text)
{
	time_t t = time(0);   // get time now
	struct tm* now = localtime(&t);
	char   tmp[MAX_LOG_SIZE];

	// 一次性获取时间
	// 并且将日志类型及敏感文件的文件名拼入同一块缓存
	sprintf(tmp, "%d-%02d-%02d %02d:%02d\n%d %s",
		now->tm_year + 1900,
		now->tm_mon + 1,
		now->tm_mday,
		now->tm_hour,
		now->tm_min,
		lt,
		pureName);


	// 构造日志
	this->message = tmp;
	this->message += text;

	return this->SendInfo(CMD_LOG, this->message.c_str());
}


bool User::EndSession()
{
	cout << "Disconnecting from Server...\n" << endl;
	
	this->SendInfo(CMD_END, "Bye-bye.");
	this->statu = STATUE_DISCONNECTED;

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


// 当服务端存在请求的文件时,为了防止获取到重复文件
// 检查服务端所存放的文件是否与本地的文件哈希值相同
// 如果不同,则说明本地不存在雷同文件,继续接受
// 如果相同,则说明本地已经存在雷同文件,通知服务端本地已存在所求文件
// 双方完成文件请求交互
/*
	为提高性能,可以改变客户端与服务端发生文件交互时,
	双方都新建一个进(线)程单独处理.

	为开发进度考虑,暂时不做多进程处理
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


bool User::UploadFile(SFile &file)
{
	cout << "Sending " << file.fileName << "to server..." << endl;
	this->SendInfo(CMD_UPD, file.fileName.c_str());
	this->SendInfo(CMD_RPL, file.fileHash.c_str());
	
	this->GetReplyInfo();

	return true;
}
