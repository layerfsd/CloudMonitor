#pragma once

#include "manage.h"

#define UPDATE_ARGS		"--backend"

#define VERSION_FILE	"DATA\\VERSION"
#define CONFIG_FILE 	"DATA\\config.ini"
#define LOCAL_HASHLIST	"DATA\\hashlist.txt"

#define DELETE_FILE_CMD		"DEL /Q /F "
#define DELETE_DIRS_CMD		"RD /Q /S " 

#define FTP_AUTH		"ftp:ftp"
#define MAXLINE			 128

#define TMP_BACKUPDIR	"bakdir_albertofwb"
#define TMP_HASHLIST	"hashlistfile_albertofwb"
#define TMPFILE_NAME	"tmp_file_albertofwb"
#define TMPDOWN_DIR		"tmpdir_albertofwb"	

#define UPDATE_CHECKED_FLAG		"UPDATE_CHECKED"

struct AppConfig
{
	char ServAddr[32];	  // Զ�̷����IPv4��ַ
	char UpdateServ[32];  // '���·�����'IPv4��ַ

	int  ServPort;		  // Զ�̷���˼����˿�
	int  LocalPort;		  // ����TCPͨ�Ŷ˿�
};

struct Config
{
	AppConfig aconfig;
	char	update_url[MAXLINE];
	bool	enableLog;
};

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

// ����һ���ļ�
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


// ȷ��Ҫ�洢��·���а��������ġ�Ŀ¼�ṹ��
void CheckPathExists(const string& curPath);

// ���ز����������ļ�
bool LoadConfig();

// �ɷ������б�����ʱ�������Ҫ����־
int WriteToLog(char* str);
void EnableLog();

bool MyCreateProcess(LPCSTR appName, LPSTR appArgs = NULL);
