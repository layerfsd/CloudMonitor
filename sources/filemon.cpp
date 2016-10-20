#include "FileMon.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

using namespace std;

int DumpToFile(char *FileName, char *buf, size_t FileSize)
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

int DumpFromFile(char *FileName, char *buf, size_t FileSize)
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

/*
关键字列表 分隔符设置为 '\n'
行数、最大行长度
*/
int ReadKeywords(const char *FileName, vector<Keyword> &kw)
{
	FILE *fp = NULL;
	char *FileBuf = NULL;
	int   TotalLine = 0;


	//GetFileSize(FileName, &FileSize);
	if ((fp = fopen(FileName, "r")) == NULL)
	{
		perror(FileName);
		return -1;
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
	return 0;
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
	char num[16];
	map<Keyword, int>::const_iterator ite;

	for (ite = MatchRecord.begin(); ite != MatchRecord.end(); ite++)
	{
		printf("%s %d\n", ite->first.word.c_str(), ite->second);
		sprintf(num, "%d#", (int)(ite->first.rank));
		message += ite->first.word;
		message += ": ";
		sprintf(num, "%d ", (int)(ite->second));
		message += num;
	}
	//cout << message << endl;
	free(FileBuf);
	return nmatch;
}

