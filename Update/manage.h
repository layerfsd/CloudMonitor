#pragma once

#include <Windows.h>	// for VOID declaration

#define VERSION_FILE	"DATA\\VERSION"
#define LOCAL_HASHLIST	"DATA\\hashlist.txt"


#define UPDATE_URL		"ftp://192.168.207.132/CloudMonitor/"
#define FTP_AUTH		"ftp:ftp"

#define TMP_HASHLIST	"hashlistfile_albertofwb"
#define TMPFILE_NAME	"tmp_file_albertofwb"


//start: http://blog.csdn.net/exlsunshine/article/details/29177025
struct FtpFile
{
	const char *filename;
	FILE *stream;
};
int DownloadFtpFile(const char* url, FtpFile &ftpfile);
//end: http://blog.csdn.net/exlsunshine/article/details/29177025

VOID StopMyService();
BOOL SendSignal2Process(DWORD dwPid, DWORD dwSig);
BOOL FindProcessPid(LPCSTR ProcessName, DWORD& dwPid);
VOID SendControlC(DWORD pid);


bool GetFilesList(const char* url, char* tempFile);

BOOL CheckCurVersion(DOUBLE &version);
