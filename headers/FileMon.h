#pragma once
#ifndef _FILEMON_H__
#define _FILEMON_H__



#define UTF_SINGLE_LEN 3
#define DELI    0x0A
#include <string>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

#define MAX_KEYWORD		256
//struct Keyword
//{
//	int		rank;
//	char	word[MAX_KEYWORD];
//};
//

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
#define MAX_KEYWORDS   100

int DumpFromFile(char *FileName, char *buf, size_t FileSize);
int DumpToFile(char *FileName, char *buf, size_t FileSize); 
int GetFileSize(const char *FileName, size_t *FileSize);
int ReadKeywords(const char *FileName, vector<Keyword> &kw);
int KeywordFilter(vector<Keyword> &kw, char *FileName, string &message);

// these functions are writen for debug others
void DumpByte(const char *str);

#endif // _FILEMON_H__
