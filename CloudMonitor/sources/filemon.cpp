#include "FileMon.h"
#include "../headers/Encrypt.h"
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
	{
		//perror(FileName);  //调试,显示具体出错信息
		return -1;
	}
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
	char  buf[MAX_KEYWORD];
	const char *FileName = filePath.c_str();

	//GetFileSize(FileName, &FileSize);
	if ((fp = fopen(FileName, "r")) == NULL)
	{
		perror(FileName);
		return false;
	}
	
	//fscanf(fp, "%d\n", &TotalLine);
	//printf("Get TotalLine: %d\n", TotalLine);

	int   line = 0;
	Keyword tp;
	char	word[MAX_KEYWORD];

	//while (line < TotalLine)
	while (!feof(fp))
	{
		memset(buf, 0, MAX_KEYWORD);
		fgets(buf, MAX_KEYWORD, fp);
		if (strlen(buf) < 2)
		{
			break;
		}
		sscanf(buf, "%d-%s\n", &tp.rank, word);
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
			//printf("keyword: [%s] wordsize[%d]\n", kw[i].word.c_str(), kw[i].word.size());
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
	message.clear();
	for (ite = MatchRecord.begin(); ite != MatchRecord.end(); ite++)
	{
		memset(tmp, 0, MAX_LOG_SIZE);
		sprintf(tmp, "%d-%s-%d ", (int)(ite->first.rank), ite->first.word.c_str(), ite->second);
		message += tmp;
	}
	message += '\n';	//关键字详情后追加一个‘换行’
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

	if (0 != GetFileSize(fileName, &fileSize))
	{
		// file not exists
		cout << "file not exists " << fileName << endl;
		return false;
	}

	char *pData = NULL;

	if ((pData = (char*)malloc(fileSize)) == NULL) {
		// alloc memory error
		return false;
	}

	MD5_Init(&c);

	if (0 != DumpFromFile(fileName, pData, fileSize))
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


//加密一个文件
bool wrapEncreytFile(SFile& sf)
{
	
	if (!EncreptFile(sf.savedPath.c_str(), sf.encPath.c_str(), sf.encPasswd.c_str()))
	{
		return false;
	}
	// 获取加密后的文件大小
	if (0 != GetFileSize(sf.encPath.c_str(), &(sf.encSize)))
	{
		cout << "GetFileSize() error!" << endl;
		return false;
	}

	return true;
}



bool initSFile(SFile &sf)
{
	/*
	经测试发现,如果在本地打开文件前,改变其文件路径编码 gbk -> utf8 会导致找不到文件
	sf.localPath = GBKToUTF8(sf.localPath.c_str());
	*/

	// 从全路径中获取文件名
	// eg: D:\work\test.docx --> test.docx
	sf.fileName = sf.localPath.substr(sf.localPath.rfind('\\') + 1);
	sf.savedPath = TMP_DIR + sf.fileName;

	// 把目标文件拷贝到临时目录
	if (!CopyFile(sf.localPath.c_str(), sf.savedPath.c_str(), FALSE))
	{
		printf("CopyFile Error: %x\n", GetLastError());
		return false;
	}


	char buf[HASH_SIZE+1];

	if (0 != GetFileSize(sf.savedPath.c_str(), &sf.fileSize))
	{
		cout << "GetFileSize() error!" << endl;
		return false;
	}

	memset(buf, 0, HASH_SIZE + 1);
	if (HashFile(sf.savedPath.c_str(), buf))
	{
		sf.fileHash = buf;
		//cout << "HashFile() " << sf.localPath << sf.fileHash << endl;
	}
	else
	{
		//cout << "HashFile() " << sf.localPath << " error" << endl;
		return false;
	}

	// 对于 txt 文件,不需要文件解析,可能需要转码
	if (sf.fileName.substr(sf.fileName.rfind('.')) == ".text")
	{
		sf.txtName = sf.fileName;
	}

	if (sf.fileName.substr(sf.fileName.rfind('.')) == ".txt")
	{
		sf.txtName = sf.fileName;
	}
	else
	{
		// 生成临时文件名 eg: test.docx.txt
		sf.txtName = sf.fileName + TXT_SUFFIX;
	}


	// 生成临时文件的路径 eg: D:\TMP\test.docx.txt
	//sf.txtPath = TMP_DIR + sf.txtName;
	sf.txtPath = TMP_DIR + sf.txtName;

	// 生成加密文件的路径 eg: TMP\test.docx.aes
	sf.encSrc = GBKToUTF8(sf.savedPath.c_str());
	sf.encName = sf.fileName + ".aes";
	sf.encPath = TMP_DIR + sf.encName;

	// 设置加密文件的密码为明文文件的哈希区间(哈希长度为32Bytes)
	sf.encPasswd = sf.fileHash.substr(6, 14);


	// 由于服务端采用 utf-8 编码，因此转换文件名编码,  gbk -> utf8 
	// 防止服务端显示上传的文件名为乱码
	sf.fileName = GBKToUTF8(sf.fileName.c_str());

	return true;
}


inline void _showSFile(SFile &sf)
{
	cout << "savedPath: " << sf.savedPath << endl;
	cout << "fileName: " << sf.fileName << endl;
	cout << "fileSize: " << sf.fileSize << endl;

	cout << "hash: " << sf.fileHash << endl;

	cout << "txtName: " << sf.txtName << endl;
	cout << "txtPath: " << sf.txtPath << endl;

	cout << "encName: " << sf.encName << endl;
	cout << "encPath: " << sf.encPath << endl;
	cout << "encSize: " << sf.encSize << endl;
}


bool fsFilter(SFile &sf, vector<Keyword> &kw, vector<HashItem> &hashList, string &message)
{	
	if (-1 ==_access(sf.localPath.c_str(), 0))
	{
		cout << sf.localPath << " not exists!!!" << endl;
		return false;
	}
	if (kw.size() <= 0)
	{
		cout << "Empty KeywordList..." << endl;
		// 字典为空
		return false;
	}

	if (!initSFile(sf))
	{
		cout << "initSfile Failed!" << endl;
		return false;
	}
	char localPath[_MAX_PATH], txtPath[_MAX_PATH];
	
	strncpy(localPath, sf.savedPath.c_str(), _MAX_PATH);
	strncpy(txtPath, sf.txtPath.c_str(), _MAX_PATH);

	ParseFile2Text(localPath, txtPath);
	if (!KeywordFilter(kw, txtPath, message))
	{
		cout << "Find nothing from: " << sf.txtPath << endl;
		return false;
	}

	wrapEncreytFile(sf);
	_showSFile(sf);

	cout << "matched: " << message << endl;
	return true;
}




bool isContinue(const char* lPath, int length)
{
	return true;
}