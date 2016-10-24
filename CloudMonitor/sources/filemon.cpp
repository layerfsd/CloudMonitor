#include "FileMon.h"
#include "parsedoc.h"

#include <windows.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

using namespace std;

int DumpToFile(const char *FileName, char *buf, size_t FileSize)
{
	FILE     *fp;

	if ((fp = fopen(FileName, "wb")) == NULL)
	{
		return -1;
	}
	fwrite(buf, 1, FileSize, fp);

	fclose(fp);

	return 0;
}

int DumpFromFile(const char *FileName, char *buf, size_t FileSize)
{
	FILE     *fp;

	if ((fp = fopen(FileName, "rb")) == NULL)
	{
		return -1;
	}
	fread(buf, 1, FileSize, fp);

	fclose(fp);

	return 0;
}


int GetFileSize(const char *FileName, size_t *FileSize)
{
	FILE *fp;

	if ((fp = fopen(FileName, "rb")) == NULL)
		return -1;
	fseek(fp, 0, SEEK_END);
	*FileSize = ftell(fp);

	fclose(fp);
	fp = NULL;

	return 0;
}


bool LoadHashList(string &path, vector<HashItem> hashList)
{
	FILE *fp = NULL;
	char *FileBuf = NULL;
	const char *FileName = path.c_str();

	if ((fp = fopen(FileName, "r")) == NULL)
	{
		perror(FileName);
		return false;
	}

	int			cnt = 0;
	HashItem	tp;

	while (!feof(fp))
	{
		cnt += 1;
		fscanf(fp, "%s %s %d\n", tp.path, tp.hash, &tp.times);
		printf("%3d path:%s hash:%s times:%d\n", cnt, tp.path, tp.hash, tp.times);
		hashList.push_back(tp);
	}

	fclose(fp);
	return true;
}


bool StoreHashList(string &path, vector<HashItem> hashList)
{
	return true;
}


bool LoadKeywords(string  &filePath, vector<Keyword> &kw)
{
	FILE *fp = NULL;
	char *FileBuf = NULL;
	int   TotalLine = 0;
	const char *FileName = filePath.c_str();

	//GetFileSize(FileName, &FileSize);
	if ((fp = fopen(FileName, "r")) == NULL)
	{
		perror(FileName);
		return false;
	}
	
	fscanf(fp, "%d\n", &TotalLine);
	printf("Get TotalLine: %d\n", TotalLine);

	int   line = 0;
	Keyword tp;
	char	word[MAX_KEYWORD];

	while (line < TotalLine)
	{
		fscanf(fp, "%d %s\n", &tp.rank, word);
		tp.word = word;
		printf("rank: %d len: [%d] word: %s\n", tp.rank, tp.word.size(), word);
		kw.push_back(tp);
		line += 1;
	}

	fclose(fp);
	return true;
}



/*
return value:
n matched items
-1 error
0 none match
*/

void DumpByte(const char *str)
{
	while (*str)
	{
		printf("%02X ", *str & 0x000000FF);
		str++;
	}
	putchar('\n');
}

int KeywordFilter(vector<Keyword> &kw, char *FileName, string &message)
{
	char    *FileBuf;
	size_t  FileSize = 0;
	size_t  offset = 0;
	int     nmatch = 0;

	if (GetFileSize(FileName, &FileSize) != 0)
	{
		printf("GetFileSize error");
		return -1;
	}
	if ((FileBuf = (char *)malloc(FileSize)) == NULL)
	{
		perror("FileBuf");
		return -1;
	}

	if (DumpFromFile(FileName, FileBuf, FileSize) != 0)
	{
		printf("DumpFileByte error");
		return -1;
	}

	offset = 0;
	int WordLen = 0;
	map<Keyword, int> MatchRecord;

	while (offset < FileSize)
	{
		for (unsigned int i = 0; i != kw.size(); i += 1)
		{
			/*
			为提高性能，此处可以自己实现一个 memcmp，以减少堆栈调用开销
			同时改变这种穷举匹配方式为更加高效的字符串匹配算法
			*/
			if (!memcmp(kw[i].word.c_str(), FileBuf + offset, kw[i].word.size()))
			{
				MatchRecord[kw[i]]++;
				nmatch += 1;
				printf("%s matched in offset %u!!!\n", kw[i].word.c_str(), offset);
			}
		}
		offset += 1;
	}

	// 生成报告日志
	//message += "File: ";
	//message += PlainFile;
//	message += " Matches ";
	char tmp[MAX_LOG_SIZE];

	map<Keyword, int>::const_iterator ite;

	for (ite = MatchRecord.begin(); ite != MatchRecord.end(); ite++)
	{
		memset(tmp, 0, MAX_LOG_SIZE);
		sprintf(tmp, " %d-%s-%d", (int)(ite->first.rank), ite->first.word.c_str(), ite->second);
		message += tmp;
	}
	//cout << message << endl;
	free(FileBuf);
	return nmatch;
}

bool HashFile(const char *fileName, char *buf)
{
	MD5_CTX c;
	unsigned char md5[17] = { 0 };
	const char *set = "0123456789abcdef";
	int cnt = 0;

	size_t fileSize = 0;

	if (!GetFileSize(fileName, &fileSize))
	{
		// file not exists
		return false;
	}

	char *pData = NULL;

	if ((pData = (char*)malloc(fileSize)) == NULL) {
		// alloc memory error
		return false;
	}

	MD5_Init(&c);

	if (!DumpFromFile(fileName, pData, fileSize))
	{
		return false;
	}
	MD5_Update(&c, pData, fileSize);

	MD5_Final(md5, &c);
	free(pData);

	/* convert md5hash to string */
	cnt = 0;
	while (cnt < MD5_DIGEST_LENGTH)
	{
		buf[cnt * 2] = set[md5[cnt] >> 4];
		buf[cnt * 2 + 1] = set[md5[cnt] & 0xF];
		cnt += 1;
	}
	/* end convert */

	return true;
}


inline bool initSFile(SFile &sf)
{
	// 从全路径中获取文件名
	// eg: D:\work\test.docx --> test.docx

	sf.localPath = GBKToUTF8(sf.localPath.c_str());
	sf.fileName = sf.localPath.substr(sf.localPath.rfind('\\') + 1);

	// 生成临时文件名 eg: test.docx.txt
	sf.txtName = sf.fileName + TXT_SUFFIX;

	// 生成临时文件的路径 eg: D:\TMP\test.docx.txt
	sf.txtPath = TMP_DIR + sf.txtName;

	// 生成加密文件的文件名 eg: test.docx.aes
	sf.encName = sf.fileName + ENC_SUFFIX;

	// 生成加密文件的路径 eg: D:\TMP\test.docx.aes
	sf.encPath = TMP_DIR + sf.encName;
	
	return true;
}


inline void _showSFile(SFile &sf)
{
	cout << "localPath: " << sf.localPath << endl;
	cout << "fileName: " << sf.fileName << endl;

	cout << "txtName: " << sf.txtName << endl;
	cout << "txtPath: " << sf.txtPath << endl;

	cout << "encName: " << sf.encName << endl;
	cout << "encPath: " << sf.encPath << endl;
}


string GBKToUTF8(const char* strGBK)
{
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}


bool fsFilter(SFile &sf, vector<Keyword> &kw, vector<HashItem> &hashList, string &message)
{	
	if (-1 ==_access(sf.localPath.c_str(), 0))
	{
		cout << sf.localPath << "not exists!!!" << endl;
		return false;
	}
	if (kw.size() <= 0)
	{
		return false;
	}

	initSFile(sf);
	_showSFile(sf);
	char localPath[_MAX_PATH], txtPath[_MAX_PATH];
	
	strncpy(localPath, sf.localPath.c_str(), _MAX_PATH);
	strncpy(txtPath, sf.txtName.c_str(), _MAX_PATH);

	ParseFile2Text(localPath, txtPath);
	if (!KeywordFilter(kw, txtPath, message))
	{
		cout << "Find nothing from: " << sf.localPath << endl;
		return false;
	}
	
	return true;
}