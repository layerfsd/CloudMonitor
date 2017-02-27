#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <io.h>
//#include "doc2txt.h"
#include "parsedoc.h"
#include "FileMon.h"

using namespace std;

struct SuffixMap
{
	char *suffix;
	int  type;
};

//int ExtractDocxFile(char *FileName, char *TextName)
int ParseAll2Txt(const char *FileName, const char *TextName)
{
	char cmd[256] = { 0 };
	FILE   *pPipe;

	if (NULL == TextName || NULL == FileName)
	{
		return -1;
	}


	//printf("[pdf] parsing %s to %s ...\n", FileName, TextName);

	snprintf(cmd, CMD_LEN, "%s -o -c%d \"%s\" \"%s\" \n", PARSE_TOOL, KEYWORD_ENCODING, FileName, TextName);
	//fputs(cmd, stdout);

	if ((pPipe = _popen(cmd, "r")) == NULL)
	{
		return -1;
	}
	else
	{
		fgets(cmd, 256, pPipe);
		//fputs(cmd, stdout);
		_pclose(pPipe);
	}
	return 0;
}

int GetFileType(const char *FileName)
{
	static SuffixMap FileTypeList[] = {
		{ ".wps",       WPS_TYPE },
		{ ".doc",		DOC_TYPE },
		{ ".docx",		DOCX_TYPE },


		{ ".text",		TEXT_TYPE },
		{ ".txt",		TEXT_TYPE },

		{ ".pdf",		PDF_TYPE },

		{ ".xls",		XLS_TYPE },
		{ ".xlsx",		XLS_TYPE },


		{ ".ppt",		PPT_TYPE },
		{ ".pptx",		PPT_TYPE },

		{".rtf",		RTF_TYPE},
	};

	const char *suffix = NULL;
	int   length = sizeof(FileTypeList) / sizeof(FileTypeList[0]);
	int   type = NONE_TYPE;

	suffix = strrchr(FileName, SEPERATOR);

	if ((NULL == suffix ) || (strlen(suffix) < MIN_SUFFIX_LEN))
	{
		return NONE_TYPE;
	}

	for (int i = 0; i < length; i++)
	{
		if (!_stricmp(suffix, FileTypeList[i].suffix))
		{
			//printf("recognizing: %s\n", FileTypeList[i].suffix);
			type = FileTypeList[i].type;
			break;
		}
	}

	return type;
}


int ParseFile2Text(const char *FileName, char *TextName)
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

	switch (FileType)
	{
	case NONE_TYPE:
		ret = -1;
		//printf("[none] parsing %s to %s ...\n", FileName, TextName);
		break;

	case TEXT_TYPE:
		if (!IsUtf8(TextName))
		{
			//зЊТы GBK ---> utf8
			DecodeGB2312ToUtf8(TextName);
		}
		printf("[text] parsing %s to %s ...\n", FileName, TextName);
		break;

	case DOC_TYPE:
		ret = ParseAll2Txt(FileName, TextName);
		//printf("[doc] parsing %s to %s ...\n", FileName, TextName);
		break;


	case WPS_TYPE:
		ret = ParseDoc(FileName, TextName);
		//printf("[wps] parsing %s to %s ...\n", FileName, TextName);
		break;


	default:
		ret = ParseAll2Txt(FileName, TextName);
		break;
	}

	return ret;
} 
