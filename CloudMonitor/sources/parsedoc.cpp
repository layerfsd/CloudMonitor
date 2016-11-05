#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <io.h>
#include "doc2txt.h"
#include "parsedoc.h"
#include "FileMon.h"

using namespace std;

struct SuffixMap
{
	char *suffix;
	int  type;
};

int ExtractDocxFile(char *FileName, char *TextName)
{
	char cmd[256] = { 0 };

	if (NULL == FileName)
	{
		return -1;
	}

	// 暂时先调用 7z.exe 执行解压缩与问价加/解密
	// 后期为提高性能可以直接调用 7z 提供的 DLL
	FILE   *pPipe;

	snprintf(cmd, CMD_LEN, "%s e -y %s %s\n", EXTRACT_TOOL, FileName, DOCX_TARGET_FILE);
	//fputs(cmd, stdout);
	if ((pPipe = _popen(cmd, "r")) == NULL)
	{
		return -1;
	}
	else
	{
		_pclose(pPipe);
	}
	//if (!_access(TextName, 0))
	//{
	//	remove(TextName);
	//}
	rename(DOCX_FILE_NAME, TextName);
	return 0;
}

int GetFileType(char *FileName)
{
	static SuffixMap FileTypeList[] = {
		{ ".doc",		DOC_TYPE },
		{ ".pdf",		PDF_TYPE },
		{ ".docx",		DOCX_TYPE },
		{ ".txt",		TEXT_TYPE },
		{ ".text",		TEXT_TYPE },
		{ ".cpp",		TEXT_TYPE },
		{ ".c",			TEXT_TYPE },
	};

	char *suffix = NULL;
	int   length = sizeof(FileTypeList) / sizeof(FileTypeList[0]);
	int   type = NONE_TYPE;

	suffix = strrchr(FileName, SEPERATOR);
	//printf("FileTypeList size: %d\n", length);
	//printf("Get Suffix: %s\n", suffix);

	if ((NULL == suffix ) || (strlen(suffix) < MIN_SUFFIX_LEN))
	{
		return NONE_TYPE;
	}

	for (int i = 0; i < length; i++)
	{
		if (!strcmp(suffix, FileTypeList[i].suffix))
		{
			//printf("recognizing: %s\n", FileTypeList[i].suffix);
			type = FileTypeList[i].type;
			break;
		}
	}

	return type;
}


int ParseFile2Text(char *FileName, char *TextName)
{
	int ret = 0;
	int FileType = 0;

	if (_access(FileName, 0))
	{
		fprintf(stderr, "%s not exist!\n", FileName);
		return -1;
	}
	FileType = GetFileType(FileName);
	//cout << FileType << endl;

#if 1
	switch (FileType)
	{
	case NONE_TYPE:
		ret = -1;
		printf("[none] parsing %s to %s ...\n", FileName, TextName);
		break;

	case TEXT_TYPE:
		printf("[text] parsing %s to %s ...\n", FileName, TextName);
		break;

	case DOCX_TYPE:
		ret = ExtractDocxFile(FileName, TextName);
		printf("[docx] parsing %s to %s ...\n", FileName, TextName);
		break;

	case DOC_TYPE:
		ret = ParseDoc(FileName, TextName);
		printf("[doc] parsing %s to %s ...\n", FileName, TextName);
		break;
	
	case PDF_TYPE:
		printf("[pdf] parsing %s to %s ...\n", FileName, TextName);
		break;

	default:
		break;
	}
#endif
	return ret;
} 
