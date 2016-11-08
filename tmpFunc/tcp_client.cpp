#include "stdafx.h"

int tcpClient()
{
#if RELEASE
	if (argc < 2)
	{
		printf("Usage: %s <filePath>\n", argv[0]);
		return -1;
	}
#endif
	SOCKET soc;
	SOCKADDR_IN serveraddr;
	char buf[1024];

	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);	//initial Ws2_32.dll by a process

	if ((soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)	//create a tcp socket
	{
		printf("Create socket fail!\n");
		return -1;
	}

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(50006);
	serveraddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	//connect to server
	printf("Try to connect...\n");
	if (connect(soc, (SOCKADDR *)&serveraddr, sizeof(serveraddr)) != 0)
	{
		printf("Connect fail!\n");
		return -1;
	}
	printf("Connected\n");

	//strncpy(buf, argv[1], 260);
	char fName[] = "F:\\NutStore\\SSL传输\\安全办公信息监控平台项目研发方案20160922.docx";
	strncpy(buf, fName, 260);
	//strncpy(buf, "d:\\test.txt", 260);


	int length = strlen(buf) + 1;

	if (send(soc, buf, length, 0) <= 0)
	{
		printf("Error!\n");
	}
	printf("send %s done!\n", buf);

	recv(soc, buf, 1024, 0);
	printf("replied: %s\n", buf);

	WSACleanup(); //clean up Ws2_32.dll
	return 0;
}
