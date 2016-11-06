#pragma once
#ifndef _FILEMON_H__
#define _FILEMON_H__


#define MAX_KEYWORD			256
#define MAX_LOG_SIZE		1024
#define MD5_STR_SIZE		32
#define HASH_SIZE			32
#define MAX_PASSWD			32

#define UTF_SINGLE_LEN 3
#define DELI    0x0A

#define TXT_SUFFIX			".txt"
#define ENC_SUFFIX			".aes"
#define TMP_DIR				"TMP\\"
#define RES_DIR				"DATA\\"

#define HASHLST_LEN			1024
#define KEYWORD_PATH		"DATA\\keywords.txt"
#define HASHLST_PATH		"DATA\\hashlist.txt"


#include <string>
#include <iostream>
#include <vector>
#include <map>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/md5.h>

#include <direct.h>
#include <io.h>

using namespace std;


enum LogType
{
	FILE_NETEDIT = 1,
	FILE_COPY2USB,
};

struct HashItem
{
	char  path[_MAX_PATH];		//路径
	char  hash[HASH_SIZE+1];		//哈希值
	int	  times;		//命中次数
};


// 重载 '<' 运算符是为了 map() 的哈希计算
// 如果不重载,则无法使用 map() 提供的`键-值` 映射功能
struct Keyword
{
	string word;
	int rank;
	bool operator<(const Keyword& test) const
	{
		if (word < test.word)
		{
			return true;
		}
		else if (word == test.word)
		{
			if (rank < test.rank)
				return true;
			else
				return false;
		}
		else
		{
			return false;
		}
	}
};


// 程序在运行时,对于一个`敏感文件`所需的相关信息
struct SFile
{
	// `原始文件`表示本软件要监控的文件类型
	// 目前包括 .doc .docx .txt
	// 后期还将支持 .rtf .cls .pdf ...
	string fileName;		// 原始文件名称	 eg: 财务报表.doc
	string localPath;		// 文件本地路径  eg: C:\Users\王老五\Desktop\账务信息\财务报表.doc
	string savedPath;		//原始文件临时保存路径  TMP\\财务报表.doc
	string fileHash;		// 原始文件哈希 (固定长度,32 bytes)
	size_t fileSize;		// 原始文件的大小 

	
	/* 对于所有`非纯文本文件`,统一先将其解析为一个`txt纯文本文件` */
	string txtName;			
	string txtPath;
	size_t txtSize;

	// 传送文件至服务器时,先将原始文件压缩再上传
	string encName;			// 原始文件加密后的名称				eg: 财务报表.doc.aes
	string encPath;			// 原始文件加密后存放的本地路径		eg: C:\Users\王老五\TMP\财务报表.doc.aes
	string encPasswd;		// 针对原始文件加密后所用的随机密码 (密码长度: 8-32 bytes)
	size_t encSize;			// 原始文件加密后的大小
};

// 文件内容过滤
bool fsFilter(SFile &file, vector<Keyword> &kw, vector<HashItem> &hashList, string &message);

// 从本地加载哈希索引
bool LoadHashList(string &path, vector<HashItem> hashList);

// 解析并读取字典文件到一个 容器
// 成功返回 0,失败返回-1
bool LoadKeywords(string &fileName, vector<Keyword> &kw);


// 工具函数 --- 计算一个文件的哈希值
bool HashFile(const char *fileName, char *buf);

// 已二进制的方式一次性读取一个文件内容到缓冲区
// 成功返回 0,失败返回-1
int DumpFromFile(const char *FileName, char *buf, size_t FileSize);

// 已二进制的方式把缓冲区一次性写入到一个文件
// 成功返回 0,失败返回-1
int DumpToFile(const char *FileName, char *buf, size_t FileSize);

// 获取文件大小
// 成功返回 0,失败返回-1
int GetFileSize(const char *FileName, size_t *FileSize);

// 返回匹配关键字个数, 0 表示没有在文件中匹配到关键字
/*  
	messsage 生成匹配详细记录
	kw		 保存着匹配字典列表
	fileName 为待匹配的文件路径 
*/
int KeywordFilter(vector<Keyword> &kw, char *filePath, string &message);

// 显示一个 buffer的十六进制形式
// 这是一个开发过程中的调试函数,正式程序中不需要
void DumpByte(const char *str);


// http://blog.csdn.net/chenjiayi_yun/article/details/45603773
// c++字符串编码GBK到UTF8的转换
string GBKToUTF8(const char* strGBK);

#endif // _FILEMON_H__
