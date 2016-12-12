#pragma once

#ifndef _PARSEDOC_H__
#define _PARSEDOC_H__


#define DOCX_TARGET_FILE	"word/document.xml"
#define DOCX_FILE_NAME		"document.xml"

#define EXTRACT_TOOL		"7-Zip\\7z.exe"
#define CMD_LEN				256

#define NONE_TYPE		0
#define DOC_TYPE		1
#define DOCX_TYPE		2
#define TEXT_TYPE		3
#define PDF_TYPE		4

#define SEPERATOR		'.'
#define MIN_SUFFIX_LEN  2
#define MAX_SUFFIX_LEN  4


#define WORK_DIRECTORY  "TMP"

#include "FileMon.h"

//int ParseFile2Text(SFile &sf);
int ParseFile2Text(char *FileName, char *TextName);
int DecodeUcs2Utf8(char *FileName);
int ParseDoc(const char *docFileName, const char *txtFileName);

#endif