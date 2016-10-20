#pragma once
#ifndef _NETWORK_H__
#define _NETWORK_H__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/md5.h>

#define TEST_FILENAME "test.docx"
#define READ_BNR_SIZE 1024*1024
#define MD5_STR_SIZE  32
#define MAXBUF 1280
#define SERV_PORT  50005
#define SERV_ADDR  "192.168.43.132"
#define CONNECT_TIMEOUT   1000

#define SSL_CHANNEL_ON  0
#define SSL_CHANNEL_OFF 1
#define SSL_WORKING     0
#define SSL_NOT_WORKING 1

#define MAX_FILENAME  128
#define MAX_SYSVER    48
#define MAX_USERNAME  32
#define HASH_SIZE     32
#define MAX_PASSWD    32
#define MAC_SIZE      20
#define IP_SIZE		  16
#define MAX_RETRY_TINE  5

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
	SSL     *ssl;
	SSL_CTX *ctx;
	SOCKET   sock;
	char     buf[MAXBUF];
};


struct Args
{
	char servIP[32];
	char sendFile[MAX_FILENAME];
	int  servPort;
};


int InitSSL(char *ip, int port);
int SSLSend(char *buf, int len);
int SSLRecv(char *buf, int len);
int EndSSL();
int SendFile(char *fileName);


char *HashFile(char *fileName);
int argsParse(char ac, char **av, Args *con);


int IsCnt2Internet();

#endif // _HEADER_H__