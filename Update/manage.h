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
	char ServAddr[32];	  // 远程服务端IPv4地址
	char UpdateServ[32];  // '更新服务器'IPv4地址

	int  ServPort;		  // 远程服务端监听端口
	int  LocalPort;		  // 本地TCP通信端口
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


// 确保要存储的路径中包含依赖的‘目录结构’
void CheckPathExists(const string& curPath);

// 加载并解析配置文件
bool LoadConfig();

// 由服务运行本程序时，输出必要的日志
int WriteToLog(char* str);
void EnableLog();

bool MyCreateProcess(LPCSTR appName, LPSTR appArgs = NULL);
