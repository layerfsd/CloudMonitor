#include <Windows.h>
#include <iostream>
#include <string>
#include <queue>
#include <vector>

#include "FileMon.h"
#include "parsedoc.h"

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
	} while (*szDrive != '\x00');

	while (!driveList.empty())
	{
		cout << driveList.front() << endl;
		FindAllFiles(collector, driveList.front());
		driveList.pop();
	}
	return true;
}


