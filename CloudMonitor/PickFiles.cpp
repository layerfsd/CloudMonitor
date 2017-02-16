#include <Windows.h>
#include <iostream>
#include <string>
#include <queue>
#include <vector>

#include "FileMon.h"
#include "parsedoc.h"
#include "patches.h"

using namespace std;

#define BUFSIZE				1024

// 判断是否为 指定格式的文件
static BOOL ScanFile(LPCSTR lpFilePath)
{
	static LPCSTR matchList[] = {
		//".rtf",

		".xls",
		".xlsx",

		".ppt",
		".pptx",

		".pdf",
		".doc",
		".docx",
		".wps",
	};


	DWORD dwMatchListLen = sizeof(matchList) / sizeof(matchList[0]);

	if (NULL == lpFilePath)
	{
		return FALSE;
	}
	//　获取文件后缀
	LPCSTR pos = strrchr(lpFilePath, '.');

	if (NULL == pos || '\\' == lpFilePath[0])
	{
		return FALSE;
	}
	
	// 忽略垃圾文件
	if (NULL != strchr(lpFilePath, '$'))
	{
		return FALSE;
	}

	BOOL retValue = FALSE;

	for (DWORD i = 0; i < dwMatchListLen; i++)
	{
		if (!strcmp(pos, matchList[i]))
		{
			retValue = TRUE;
			break;
		}
	}

	return retValue;
}

// 把指定路径下的所有符合‘特定后缀’的文件路径加入容器
static void FindAllFiles(vector<string>& collector, string& folderName)
{
	WIN32_FIND_DATA FileData;
	string folderNameWithSt = folderName + "*";
	string CurPath;
	HANDLE FirstFile = FindFirstFile(folderNameWithSt.c_str(), &FileData);

	if (FirstFile != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(FileData.cFileName, ".") == 0 || strcmp(FileData.cFileName, "..") == 0)
			{
				// 忽略特殊目录
				continue;
			}
			// 遇到目录则递归
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				string NewPath = folderName + FileData.cFileName;
				NewPath = NewPath + "\\";

				FindAllFiles(collector, NewPath);
			}
			// 遇到普通文件则扫描
			else
			{
				// 拼接文件的全路径
				CurPath = folderName + FileData.cFileName;
				// 扫描该文件
				if (ScanFile(CurPath.c_str()))
				{
					//cout << CurPath << endl;
					collector.push_back(CurPath);
				}
			}

		} while (FindNextFile(FirstFile, &FileData));
	}

	return;
}


bool PickLocalPath(vector<string>& collector)
{
	// 保存本地盘符
	queue<string> driveList;

	CHAR szLogicDriveStrings[BUFSIZE];
	PCHAR szDrive;
	ZeroMemory(szLogicDriveStrings, BUFSIZE);
	GetLogicalDriveStrings(BUFSIZE - 1, szLogicDriveStrings);
	szDrive = (PCHAR)szLogicDriveStrings;

	do
	{
		driveList.push(szDrive);
		szDrive += (lstrlen(szDrive) + 1);

		// 为测试方便，暂时只扫描本地的第一个盘符
		break;
	} while (*szDrive != '\x00');

	while (!driveList.empty())
	{
		cout << driveList.front() << endl;
		FindAllFiles(collector, driveList.front());
		driveList.pop();
	}
	return true;
}

extern vector<HashItem> hashList;
extern vector<Keyword> kw;

bool ScanLocalFiles(vector<Match>& scanResults)
{
	Match	tFile;		// 记录临时文件日志
	SFile	file;		// 记录临时文件信息					
	string  tMatch;		// 记录临时匹配详情

	vector<string> collector;	// 保存本地硬盘的所有符合后缀的文件

								// 检索出本地硬盘的所有“特定后缀”格式的文件
	PickLocalPath(collector);

	for (size_t i = 0; i < collector.size(); i++)
	{
		memset(&tFile, 0, sizeof(tFile));

		file.localPath = collector[i];
		// 判断是否为涉密文件
		if (fsFilter(file, kw, hashList, tMatch, tFile.keywordContext))
		{
			strncpy(tFile.fullPath, file.utf8Path.c_str(), 256);
			strncpy(tFile.matchDetail, tMatch.c_str(), 512);
			scanResults.push_back(tFile);
		}
	}

	return true;
}

bool RemoteScanLocalFiles(string& message, string& args)
{
	vector<Match> scanResults;	// 保存本地全盘扫描的结果

	if (!ScanLocalFiles(scanResults))
	{
		return false;
	}

	// 处理结果为空的情况
	if (scanResults.size() < 1)
	{
		message = "This computer is clean";
		return false;
	}

	for (size_t i = 0; i < scanResults.size(); i++)
	{
		message += scanResults[i].fullPath;
		message += "|";
		message += scanResults[i].matchDetail;
		message += "|";
		message += scanResults[i].keywordContext;
		message += "\n";
	}

	system("del /F /S /Q TMP"); 	// 删除所有临时文件

	return true;
}
