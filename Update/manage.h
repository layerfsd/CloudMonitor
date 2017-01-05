#pragma once

#include "manage.h"

#define VERSION_FILE	"DATA\\VERSION"
#define LOCAL_HASHLIST	"DATA\\hashlist.txt"

#define DELETE_FILE_CMD		"DEL /Q /F "
#define DELETE_DIRS_CMD		"RD /Q /S " 

#define UPDATE_URL		"ftp://192.168.207.132/CloudMonitor/"
#define FTP_AUTH		"ftp:ftp"

#define TMP_HASHLIST	"hashlistfile_albertofwb"
#define TMPFILE_NAME	"tmp_file_albertofwb"
#define TMPDOWN_DIR		"tmpdir_albertofwb"	


struct HashItem
{
	char fileName[MAX_PATH];
	char md5[33];
};

//start: http://blog.csdn.net/exlsunshine/article/details/29177025
struct FtpFile
{
	const char *filename;
	FILE *stream;
};

// 下载一个文件
bool DownloadFtpFile(const char* url, FtpFile &ftpfile);
//end: http://blog.csdn.net/exlsunshine/article/details/29177025

VOID StopMyService();
BOOL SendSignal2Process(DWORD dwPid, DWORD dwSig);
BOOL FindProcessPid(LPCSTR ProcessName, DWORD& dwPid);
VOID SendControlC(DWORD pid);


bool GetFilesList(const char* url, char* tempFile);

BOOL CheckCurVersion(DOUBLE &version);

bool LoadHashList(const char *FileName, map<string, string>& hashList);
void SetWorkPath(char *workPath);

void IsFileExists(vector<string>& fileList);

void DeleteFiles(vector<string>& pathList);

bool GetFileMd5(string& Path, string& fileMd5);

bool IsFileHashEqual(string& path, string& hash);