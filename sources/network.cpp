#include "network.h"
#include "FileMon.h"

static SSL_Handler hdl = { 0 };

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


int SSLSend(char *buf, int len)
{
	return SSL_write(hdl.ssl, buf, len);
}


int SSLRecv(char *buf, int len)
{
	return SSL_read(hdl.ssl, buf, len);
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



int SendFile(char *fileName)
{
	char    buf[MAXBUF];
	int     ret = 0;
	size_t     fsize = 0;
	FILE*   fp = NULL;

	GetFileSize(fileName, &fsize);
	while (fsize) {
		printf("resume %d bytes\n", fsize);
		ret = fread(buf, 1, MAXBUF, fp);
		fsize -= ret;
		SSLSend(buf, ret);
	}
	printf("resume %d bytes\n", fsize);
	printf("transform %s done.\n", fileName);

	return 0;
}



char *HashFile(char *fileName)
{
	FILE *fd;
	MD5_CTX c;
	int len;
	unsigned char md5[17] = { 0 };
	const char *set = "0123456789abcdef";
	static char buf[MD5_STR_SIZE + 1] = { 0 };
	int cnt = 0;

	if (NULL == (fd = fopen(fileName, "rb"))) {
		return NULL;
	}
	unsigned char *pData = (unsigned char*)malloc(READ_BNR_SIZE);
	if (!pData) {
		return NULL;
	}
	MD5_Init(&c);

	while (0 != (len = fread(pData, 1, READ_BNR_SIZE, fd)))
	{
		MD5_Update(&c, pData, len);
	}
	MD5_Final(md5, &c);
	fclose(fd);
	free(pData);

	/* convert md5hash to string */
	while (cnt < MD5_DIGEST_LENGTH)
	{
		buf[cnt * 2] = set[md5[cnt] >> 4];
		buf[cnt * 2 + 1] = set[md5[cnt] & 0xF];
		cnt += 1;
	}
	buf[MD5_STR_SIZE] = 0;
	/* end convert */
	return buf;
}


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