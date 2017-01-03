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

struct HashItem
{
	char	path[_MAX_PATH];		// ·��
	char	hash[HASH_SIZE+1];		// ��ϣֵ
	char	matchDetails[512];	// �ļ��ؼ���ƥ������
};

// ��¼�ļ��Ĺؼ���ƥ�����
struct Match
{
	char    fullPath[256];
	char    matchDetail[512];
};

// ���� '<' �������Ϊ�� map() �Ĺ�ϣ����
// ���������,���޷�ʹ�� map() �ṩ��`��-ֵ` ӳ�书��
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


// ����������ʱ,����һ��`�����ļ�`����������Ϣ
struct SFile
{
	// `ԭʼ�ļ�`��ʾ������Ҫ��ص��ļ�����
	// Ŀǰ���� .doc .docx .txt
	// ���ڻ���֧�� .rtf .cls .pdf ...
	string fileName;		// ԭʼ�ļ�����	 eg: ���񱨱�.doc
	string localPath;		// �ļ�����·��  eg: C:\Users\������\Desktop\������Ϣ\���񱨱�.doc
	string utf8Path;		// �ļ�����·��  eg: C:\Users\������\Desktop\������Ϣ\���񱨱�.doc
	string savedPath;		//ԭʼ�ļ���ʱ����·��  TMP\\���񱨱�.doc
	string fileHash;		// ԭʼ�ļ���ϣ (�̶�����,32 bytes)
	size_t fileSize;		// ԭʼ�ļ��Ĵ�С 

	
	/* ��������`�Ǵ��ı��ļ�`,ͳһ�Ƚ������Ϊһ��`txt���ı��ļ�` */
	string txtName;			
	string txtPath;
	size_t txtSize;

	// �����ļ���������ʱ,�Ƚ�ԭʼ�ļ�ѹ�����ϴ�
	string encSrc;			// ���Է���,openssl ��֧��gbk��ʽ���ļ�·��
	string encName;			// ԭʼ�ļ����ܺ������				eg: ���񱨱�.doc.aes
	string encPath;			// ԭʼ�ļ����ܺ��ŵı���·��		eg: C:\Users\������\TMP\���񱨱�.doc.aes
	string encPasswd;		// ���ԭʼ�ļ����ܺ����õ�������� (���볤��: 8-32 bytes)
	size_t encSize;			// ԭʼ�ļ����ܺ�Ĵ�С
};

// �ļ����ݹ���
bool fsFilter(SFile &file, vector<Keyword> &kw, vector<HashItem> &hashList, string &message);

// �ӱ��ؼ��ع�ϣ����
bool LoadHashList(string &path, vector<HashItem> hashList);

// ��������ȡ�ֵ��ļ���һ�� ����
// �ɹ����� 0,ʧ�ܷ���-1
bool LoadKeywords(string &fileName, vector<Keyword> &kw);


// ���ߺ��� --- ����һ���ļ��Ĺ�ϣֵ
bool HashFile(const char *fileName, char *buf);

// �Ѷ����Ƶķ�ʽһ���Զ�ȡһ���ļ����ݵ�������
// �ɹ����� 0,ʧ�ܷ���-1
int DumpFromFile(const char *FileName, char *buf, size_t FileSize);

// �Ѷ����Ƶķ�ʽ�ѻ�����һ����д�뵽һ���ļ�
// �ɹ����� 0,ʧ�ܷ���-1
int DumpToFile(const char *FileName, char *buf, size_t FileSize);

// ��ȡ�ļ���С
// �ɹ����� 0,ʧ�ܷ���-1
int GetFileSize(const char *FileName, size_t *FileSize);

// ����ƥ��ؼ��ָ���, 0 ��ʾû�����ļ���ƥ�䵽�ؼ���
/*  
	messsage ����ƥ����ϸ��¼
	kw		 ������ƥ���ֵ��б�
	fileName Ϊ��ƥ����ļ�·�� 
*/
int KeywordFilter(vector<Keyword> &kw, char *filePath, string &message);

// ��ʾһ�� buffer��ʮ��������ʽ
// ����һ�����������еĵ��Ժ���,��ʽ�����в���Ҫ
void DumpByte(const char *str);


// http://blog.csdn.net/chenjiayi_yun/article/details/45603773
// c++�ַ�������GBK��UTF8��ת��
string GBKToUTF8(const char* strGBK);

bool IsUtf8(const char* FileName);


// �ж��ļ����ͣ������Ƿ�Ҫɨ����ļ�
bool isContinue(const char* lPath, int length);

int DecodeGB2312ToUtf8(const char* FileName);

bool GetNameFromPath(const char* src, char* dst);

//����һ���ļ�
bool wrapEncreytFile(SFile& sf);

#endif // _FILEMON_H__