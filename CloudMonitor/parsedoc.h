#pragma once

#ifndef _PARSEDOC_H__
#define _PARSEDOC_H__


#define DOCX_TARGET_FILE	"word/document.xml"
#define DOCX_FILE_NAME		"document.xml"

#define EXTRACT_TOOL		"7-Zip\\7z.exe"
#define PARSE_TOOL			"all2txt\\a2tcmd.exe"

#define CMD_LEN				256

enum FILE_TYPE
{
	NONE_TYPE,

	WPS_TYPE,

	DOC_TYPE,
	DOCX_TYPE,

	TEXT_TYPE,

	PDF_TYPE,

	PPT_TYPE,

	XLS_TYPE,

	RTF_TYPE,
};



#define SEPERATOR		'.'
#define MIN_SUFFIX_LEN  2
#define MAX_SUFFIX_LEN  4

#define KEYWORD_ENCODING 65001

#define WORK_DIRECTORY  "TMP"

#include "FileMon.h"

//int ParseFile2Text(SFile &sf);
int ParseFile2Text(const char *FileName, char *TextName);
int DecodeUcs2Utf8(const char *FileName);
int ParseDoc(const char *docFileName, const char *txtFileName);

int ParseAll2Txt(const char *FileName, const char *TextName);

#endif