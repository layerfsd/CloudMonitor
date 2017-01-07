#include "network.h"
#include "ReadConfig.h"
#include "PickFiles.h"
#include "FileMon.h"
#include "process.h"  // 远程控制接口函数声明
#include "LocalTCPServer.h"

#include <queue>
#include <map>


AppConfig GS_acfg{};

static SSL_Handler hdl = { 0 };

// 根据心跳间隔时间和每次休眠时间
// 计算出客户端发送一次`心跳` 的循环次数
static int	   SLEEP_TIMES_PER_HBT = (1000 / LOOP_SLEEP_TIME) * HEART_BEAT_TIME;


bool InformUser(int info);		//patches.cpp

namespace session
{
	// 定义控制编码对应的处理函数
	struct FuncList
	{
		int			ctl;			// 控制编码
		ProcessFunc func;			// 控制函数:typedef bool(*ProcessFunc)(string& logMsg, string& args);
		char	    funcDesc[32];   // 控制函数描述
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

	// 定义远程控制的包头为 `CTL`
	const char*     CMD_CONTROL = "CTL";

	//命令返回类型:成功/失败
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
		{2, RemoteKillProcess, "KillProcess" },
		
		{3, RemoteScanLocalFiles, "Scan local files" },

		{4, RemoteShutdownNetwork, "Shutdown network"},

	};
	// 定义远程控制接口数量
	const int	    CTL_SUPPORT_NUM = sizeof(funcList) / sizeof(funcList[0]);

	//static RemoteControl ctlPac[] =
	//{
	//	{0, CTL_PROCESS_LIST, NULL}, // get process list
	//	{0, CTL_END_SESSION, NULL}  // end session
	//};



	const char*		FILE_KEEP_DIR	= "DATA\\";
	const char*		INVALID_ACCOUNT = "INVALID";
	const char*     WRONG_PASSWD	= "WRONG PASSWD";

	const char*		AUTH_FAILED		= "FAILED";
	const char*		AUTH_SUCCESS	= "OK";
	const char*		DNF_SUCCESS		= "OK";


	const int		WRONG_PASSWD_LEN	= strlen(WRONG_PASSWD);
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


bool GetServerAddress(char *servAddr, int *servPort)
{
	strcpy(servAddr, SERV_ADDR);
	*servPort = SERV_PORT;

	return true;
}

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
			Sleep(ret);
			printf("reconnect to %s:%d %d/%d times...\n", ip, port, cnt, MAX_RETRY_TINE);
		}
		else {
			break;
		}
	}
	if (cnt >= MAX_RETRY_TINE) {
		printf("remote server is not listening on %s:%d\n", SERV_ADDR, SERV_PORT);
		InformUser(CONNECT_FAILED);
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


int IsCnt2Internet(LPCSTR lpsIP, DWORD dwPort)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKADDR_IN addrSrv;
	int err;
	int ret;

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

	unsigned long NonBlock = 1;

	int ReceiveTimeout = 1500;
	setsockopt(sockClient, SOL_SOCKET, SO_RCVTIMEO, (char*)&ReceiveTimeout, sizeof(int));

	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons((short)dwPort);

	inet_pton(AF_INET, lpsIP, &addrSrv.sin_addr);
	ret = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));


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


void User::KeepAlive()
{
	if (this->statu != STATUE_DISCONNECTED)
	{
		return;
	}

	this->Authentication();
}


bool User::GetFromServer()
{
	static	FD_SET fdRead;
	int		nRet = 0;//记录发送或者接受的字节数
	static TIMEVAL	tv = { 0, 500 };//设置超时等待时间


	if (STATUE_DISCONNECTED == this->statu)
	{
		//cout << "[Error] Disconnected to Server!\n" << endl;
		return false;
	}


	FD_ZERO(&fdRead);
	FD_SET(hdl.sock, &fdRead);



	//只处理read事件，不过后面还是会有读写消息发送的
	nRet = select(0, &fdRead, NULL, NULL, &tv);

	if (nRet == 0)
	{//没有连接或者没有读事件
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

	// 遍历任务队列
	for (size_t i = 0; i < this->taskList.size(); i++)
	{
		//if
		if (this->taskList[i].notExecuted && 0 == this->taskList[i].time) // 如果任务尚未执行,则检查控制指令是否要求立即执行
		{
			/*  纠结于任务执行结束后,对任务队列采取的处理方式:
			*		1. 从任务队列中删除该任务
			*		2. 不删除该任务,仅标记该任务已成功执行
			*   起初选择方式1,后考虑到审计功能,采用方式2
			*   这样可以支持服务器查找对应主机的任务执行状况
			*/

			// 传送User类中的message引用给远程处理函数,远程处理函数产生的处理结果存储在 message 中
			cout << "Executing task ..." << endl;
			execStatus = this->taskList[i].func(this->message, taskList[i].ctlDetails);	
			if (execStatus)
			{
				// 任务成功执行,通知服务端处理结果 
				this->SendInfo(CTL_RPL_OK, message.c_str());							
			}
			else
			{
				// 任务执行失败,也通知服务端处理结果 
				this->SendInfo(CTL_RPL_FAILED, message.c_str());						
			}
			this->taskList[i].notExecuted = false;			 // 任务执行后,标记任务`未执行状态`为假
			break;
		//end if
		}
	//end for
	}
	return execStatus;
}

User::User(const char *userName)
{

	// 在程序运行时，创建一个‘标志文件’。
	// 必须调用fclose()，否则在析构函数中的 remove() 调用会因权限问题而无法删除这个临时文件
	FILE *fp = fopen(RUNNING_FLAG, "w");
	fclose(fp);

	this->statu = STATUE_DISCONNECTED;
	
	memset(&GS_acfg, 0, sizeof(GS_acfg));
	memset(tmpBuf, 0, sizeof(tmpBuf));

	LoadConfig(CONFIG_PATH, MyParseFunc, &GS_acfg);

	this->workDir = FILE_KEEP_DIR;

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

	if (0 == reveivedSize)
	{
		return true;
	}
	if (reveivedSize != HEAD_SIZE)
	{
		std::cout << "Receiving Failed!\n" << endl;
		cout << "receivedSize: " << reveivedSize << endl;
		this->statu = STATUE_DISCONNECTED;
		return false;
	}
	//printf("restPktSize: %d\n", restPktSize);
	SSL_read(hdl.ssl, pkt.text, restPktSize);


	// 判断指令类型: 是否为"远程控制"
	if (!strncmp(pkt.cmd, CMD_CONTROL, CMD_CONTROL_LEN)) // 如果收到了一条远程控制指令
	{
		pkt.text[CTL_CONTROL_PLEN] = 0;
		// 指令找到对应的处理函数
		int instructionCode = atoi(pkt.text);

		// 判断指令编码是否支持
		if ((instructionCode >= 0) && (instructionCode < CTL_SUPPORT_NUM))
		{
			memset(&tmpTask, 0, sizeof(tmpTask));				  // 初始化远程控制指令结构体	
			strncpy(tmpTask.ctlTxt, pkt.text, CTL_CONTROL_PLEN);  // 解析服务端发送的指令编号
			tmpTask.ctlDetails = pkt.text + CTL_CONTROL_PLEN + 1;	  // 解析服务端发送的指令对应的指令参数
			//cout << "details: [" << tmpTask.ctlDetails << "]" << endl;
			tmpTask.time = 0;									  // 默认所有任务都是立即执行,不等待
			tmpTask.notExecuted = true;							  // 设定任务的`未执行`状态为真

			if (0 == instructionCode)							// 如果服户端要求断开连接,则直接处理
			{
				this->ShowCmdDetail();							// 输出服务端指令
				this->statu = STATUE_DISCONNECTED;				// 设置当前会话状态为: 断开连接
				return true;
			}
			tmpTask.func = funcList[instructionCode].func;		// 为每一个任务选择对应的处理过程
			this->taskList.push_back(tmpTask);					// 将远程控制任务加入本地任务队列

			cout << "Get new task: {" << endl
				<< "\ttime: " << tmpTask.time << endl
				<< "\tnotExecuted: " << tmpTask.notExecuted << endl
				<< "\tinstructionCode: " << instructionCode << endl
				<< "\tfunc at: " << tmpTask.func << endl
				<< "\tfuncDesc: " << funcList[instructionCode].funcDesc << endl;
			cout << "}" << endl;
			return true;
		}
		else // 如果远程控制编码不支持,则通知服务端此条控制信息为非法指令
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

	if (ret < 0)
	{
		this->statu = STATUE_DISCONNECTED;
	}
	cout << "***************>CLIENT BEGIN<********************" << endl;
	cout << "[CMD] " << cmdType << endl;
	cout << "[TXT] " << text << " [textLen] " << textLen << endl;
	cout << "***************> CLIENT END <********************\n" << endl;
	//printf("[SSL_write RETURN] %d\n", ret);
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



string FormatMAC();
string FormatHDS();

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
	this->macList = FormatMAC();
	this->hdsList = FormatHDS();

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
	if (0 != InitSSL(GS_acfg.ServAddr, GS_acfg.ServPort))
	{
		return false;
	}

	this->statu = STATUE_CONNECTED;

	GetReplyInfo();
	if (!RecognizeServer(pkt.text))
	{
		cout << "Invalid Server!" << endl;
		return false;
	}
	
	// 尝试登陆
	cout << "SENDING " << userName << endl;
	SendInfo(CMD_ATH, userName);
	GetReplyInfo();


	// 检查密码
	if (!memcmp(pkt.text, WRONG_PASSWD, WRONG_PASSWD_LEN))
	{
		cout << "Wrong Passwd " << this->userName;
		InformUser(INVALID_PASSWD);
		return false;
	}

	// 检查 mac 是否与注册时的一致
	const char* mac_diff = "MAC_DIFF";
	if (!memcmp(mac_diff, pkt.text, strlen(mac_diff)))
	{
		cout << mac_diff << endl;
		InformUser(MAC_DIFF);
		return false;
	}

	// 首次登陆，需要注册
	const char* needReg = "NEED REGIST";
	if (!memcmp(needReg, pkt.text, strlen(needReg)))
	{
		cout << "Registering " << endl;
		if (!RegisterClient())
		{
			// 注册失败
			InformUser(CONNECT_FAILED);
		}
	}

	// 登录成功
	InformUser(CONNECT_SUCCESS);
	return true;
}


bool User::SendLog(const char* fHash, const char* text, int logType)
{
	time_t t = time(0);   // get time now
	struct tm* now = localtime(&t);
	char   tmp[MAX_LOG_SIZE];

	memset(tmp, 0, sizeof(tmp));

	if (USB_PLUG_EVENT == logType)
	{
		fHash = "abcd1234";
	}
	
	// 获取时间
	// 将时间和文件哈希拼入同一块缓存
	snprintf(tmp, MAX_LOG_SIZE, "%d-%02d-%02d %02d:%02d\n%s\n%s\n%d ",
		now->tm_year + 1900,
		now->tm_mon + 1,
		now->tm_mday,
		now->tm_hour,
		now->tm_min,
		fHash,
		text,
		logType);


	// 构造日志
	this->message.clear();
	this->message = tmp;	
	if (0 == logType)
	{
		this->message += "HostOnline";	// 追加 (关键字信息+日志详情)
	}


	return this->SendInfo(CMD_LOG, this->message.c_str());
}


bool User::GetRegistInf()
{
	return false;
}

bool User::EndSession()
{
	cout << "Disconnecting from Server...\n" << endl;
	if (this->statu == STATUE_DISCONNECTED)
	{
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
	if (FileName == "keywords.txt")
	{
		FileName.insert(0, FILE_KEEP_DIR);
	}
	cout << "[keyword hash] " << this->pkt.text << endl;
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

	string saveFilePath = FileName;

	if (fileSize <= 0)
	{
		cout << "Invalid file size " << fileSize << endl;
		return false;
	}
	FILE *fp = NULL;

	if ((fp = fopen(saveFilePath.c_str(), "wb")) == NULL)
	{
		cout << "Create [" << saveFilePath << "] error" << endl;
		return false;
	}

	int restSize = fileSize;
	int recvSize = 0;

	printf("Begin receiving %d bytes\n", recvSize);
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
	this->SendInfo(CMD_UPD, file.encName.c_str());
	this->SendInfo(CMD_RPL, file.fileHash.c_str());
	
	this->GetReplyInfo();
	if (!IsServerWantThis(pkt.text))
	{
		// 如果服务端已经存在当前文件了
		return true;
	}
	// 发送文件加密密码
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
	// 调试,计算发送次数
	int cnt = 0;
	while (restSize > 0)
	{
		//cout << "Remaining " << restSize << " bytes to transform..." << endl;
		memset(tmpBuf, 0, sizeof(tmpBuf));
		readSize = fread(tmpBuf, 1, MAXBUF, fp);
		//cout << "sending " << readSize << "bytes " << endl;
		// 读多少,发多少
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
	static int count = 0;   // 设计一个静态计数器来控制心跳发送

	if (count >= SLEEP_TIMES_PER_HBT)
	{
		count = 0;
		return this->SendInfo(CMD_HBT, CMD_HBT);
	}

	//cout << "LOOP_SLEEP_TIME: " << count << endl;

	this->KeepAlive();  // 当检测到客户端掉线时，尝试重新连接

	count += 1;
	Sleep(LOOP_SLEEP_TIME);

	return true;
}
