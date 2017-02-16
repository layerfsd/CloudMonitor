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

// �ж��Ƿ�Ϊ ָ����ʽ���ļ�
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
	//����ȡ�ļ���׺
	LPCSTR pos = strrchr(lpFilePath, '.');

	if (NULL == pos || '\\' == lpFilePath[0])
	{
		return FALSE;
	}
	
	// ���������ļ�
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

// ��ָ��·���µ����з��ϡ��ض���׺�����ļ�·����������
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
				// ��������Ŀ¼
				continue;
			}
			// ����Ŀ¼��ݹ�
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				string NewPath = folderName + FileData.cFileName;
				NewPath = NewPath + "\\";

				FindAllFiles(collector, NewPath);
			}
			// ������ͨ�ļ���ɨ��
			else
			{
				// ƴ���ļ���ȫ·��
				CurPath = folderName + FileData.cFileName;
				// ɨ����ļ�
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
	// ���汾���̷�
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

		// Ϊ���Է��㣬��ʱֻɨ�豾�صĵ�һ���̷�
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
	Match	tFile;		// ��¼��ʱ�ļ���־
	SFile	file;		// ��¼��ʱ�ļ���Ϣ					
	string  tMatch;		// ��¼��ʱƥ������

	vector<string> collector;	// ���汾��Ӳ�̵����з��Ϻ�׺���ļ�

								// ����������Ӳ�̵����С��ض���׺����ʽ���ļ�
	PickLocalPath(collector);

	for (size_t i = 0; i < collector.size(); i++)
	{
		memset(&tFile, 0, sizeof(tFile));

		file.localPath = collector[i];
		// �ж��Ƿ�Ϊ�����ļ�
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
	vector<Match> scanResults;	// ���汾��ȫ��ɨ��Ľ��

	if (!ScanLocalFiles(scanResults))
	{
		return false;
	}

	// ������Ϊ�յ����
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

	system("del /F /S /Q TMP"); 	// ɾ��������ʱ�ļ�

	return true;
}
