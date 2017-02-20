#include "stdafx.h"

using namespace std;


// 以下三个进程，需要在重启更新程序运行时重新启动的
LPCSTR TargetProcessName[]{ "CloudMonitor.exe", "MonitorService.exe", "MonitorService-64.exe" };

Config GlobalConfig;

void logLastError()
{
	LPTSTR errorText = NULL;

	FormatMessage(
		// use system message tables to retrieve error text
		FORMAT_MESSAGE_FROM_SYSTEM
		// allocate buffer on local heap for error text
		| FORMAT_MESSAGE_ALLOCATE_BUFFER
		// Important! will fail otherwise, since we're not 
		// (and CANNOT) pass insertion parameters
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&errorText,  // output 
		0, // minimum size for output buffer
		NULL);   // arguments - see note 

	if (NULL != errorText)
	{
		printf("failure: %s", errorText);
		LocalFree(errorText);
		errorText = NULL;
	}
}


VOID StopMyService()
{
	DWORD	dwPid = 0;
	BOOL	bRet = FALSE;
	LPCSTR  pos{ nullptr };


	for (int i = 0; i < ArraySize(TargetProcessName); i++)
	{
		pos = TargetProcessName[i];
		bRet = FindProcessPid(pos, dwPid);

		// 仅当找到进程pid时，才尝试关闭
		while (0 != bRet)
		{
			printf("Sending SIGINT to [%s] [%d]\n", pos, dwPid);
			SendControlC(dwPid);
			Sleep(2 * 1000);

			cout << "After sent SIGINT" << endl;
			bRet = FindProcessPid(pos, dwPid);
		}
	}

	return;
}

VOID SendControlC(DWORD pid)
{
	printf("sending ctrl+c to pid %d", pid);
	FreeConsole();
	if (AttachConsole(pid))
	{
		SetConsoleCtrlHandler(NULL, true);
		GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
	}
	else {
		logLastError();
	}
}

BOOL FindProcessPid(LPCSTR ProcessName, DWORD& dwPid)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return(FALSE);
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}

	BOOL	bRet = FALSE;
	do
	{
		if (!strncmp(ProcessName, pe32.szExeFile, MAX_PATH))
		{
			dwPid = pe32.th32ProcessID;
			bRet = TRUE;
			break;
		}

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return bRet;
}


static size_t GetFilesList_response(void *ptr, size_t size, size_t nmemb, void *data)
{
	FILE *writehere = (FILE *)data;
	return fwrite(ptr, size, nmemb, writehere);
}

bool GetFilesList(const char* url, char* tempFile)
{
	CURL *curl;
	CURLcode res;
	FILE *ftpfile;

	/* local file name to store the file as */
	ftpfile = fopen(tempFile, "wb"); /* b is binary, needed on win32 */

	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERPWD, FTP_AUTH);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, ftpfile);
		curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1);

		res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
	}

	fclose(ftpfile);


	if (CURLE_OK != res)
		return false;

	return true;
}




//start: http://blog.csdn.net/exlsunshine/article/details/29177025
static size_t FetchFiles(void *buffer, size_t size, size_t nmemb, void *stream)
{
	struct FtpFile *out = (struct FtpFile *)stream;
	if (out && !out->stream)
	{
		// open file for writing   
		out->stream = fopen(out->filename, "wb");
		if (!out->stream)
			return -1; // failure, can't open file to write  
	}
	return fwrite(buffer, size, nmemb, out->stream);
}

bool DownloadFtpFile(const char* url, FtpFile &ftpfile)
{
	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	bool bRet = true;

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERPWD, FTP_AUTH);
		// Define our callback to get called when there's data to be written //  
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FetchFiles);
		// Set a pointer to our struct to pass to the callback //  
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);

		// Switch on full protocol/debug output //  
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);

		// always cleanup   
		curl_easy_cleanup(curl);

		if (CURLE_OK != res)
		{
			//we failed   
			fprintf(stderr, "curl told us %d\n", res);
			bRet = false;
		}
	}

	if (ftpfile.stream)
		fclose(ftpfile.stream); // close the local file   

	curl_global_cleanup();

	return bRet;
}
//end: http://blog.csdn.net/exlsunshine/article/details/29177025

bool LoadHashList(const char *FileName, map<string, string>& hashList)
{
	FILE *fp = NULL;
	char *FileBuf = NULL;

	if ((fp = fopen(FileName, "r")) == NULL)
	{
		perror(FileName);
		return false;
	}

	HashItem	tp;
	string		tpName;

	while (!feof(fp))
	{
		memset(&tp, 0, sizeof(tp));
		fscanf(fp, "%s  %s", tp.md5, tp.fileName);
		if (strnlen(tp.fileName, sizeof(tp.fileName)) > 0 && strnlen(tp.md5, 32) > 0)
		{
			if ('.' == tp.fileName[0] && '/' == tp.fileName[1])
			{
				tpName = tp.fileName + 2;
			}
			else
			{
				tpName = tp.fileName;
			}
			hashList[tpName] = tp.md5;
			//printf("md5:%s  name:%s\n", hashList[tpName].c_str(), tpName.c_str());
		}
	}

	fclose(fp);
	return true;
}

// 设定程序工作目录
void SetWorkPath(char *workPath)
{
	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //得到当前模块路径
	strcat(strModule, "\\..\\");     //设置为当前工作路径为当时的上一级
	SetCurrentDirectory(strModule);
	GetModuleFileName(NULL, workPath, MAX_PATH); //得到当前模块路径
}

void DeleteFiles(vector<string>& pathList)
{
	struct stat s;
	string cmd;
	const char *CurPath;

	for (auto i : pathList)
	{
		CurPath = i.c_str();

		if (stat(CurPath, &s) == 0)
		{
			if (s.st_mode & S_IFDIR)
			{
				cmd = DELETE_DIRS_CMD;
				cmd += CurPath;
			}
			else if (s.st_mode & S_IFREG)
			{
				cmd = DELETE_FILE_CMD;
				cmd += CurPath;
			}
			cout << "removing " << CurPath << endl;
			system(cmd.c_str());
		}
	}
	printf("remove finished\n");
	return;
}


bool GetFileMd5(string& Path, string& fileMd5)
{
	const char* filePath = Path.c_str();
	if (_access(filePath, 0) != 0)
	{
		return false;	// 文件不存在
	}

	char* CMD_FMT = "tools\\openssl.exe md5  \"%s\"";	//防止文件路径中包含空格
	char  cmd[_MAX_PATH];
	FILE* execfd = NULL;

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, CMD_FMT, filePath);
	//printf("cmd: [%s]\n", cmd);

	execfd = _popen(cmd, "r");

	if (NULL == execfd)
	{
		return false;
	}
	else
	{
		fgets(cmd, 256, execfd);
		// MD5(openssl.exe)= 004710323e580fbd9f5f9625f363967d
		//fputs(cmd, stdout);
		
		// 找到第一行末尾
		char *tail = strchr(cmd, '\n');
		char *pre = strrchr(cmd, ' ');

		if (NULL != pre)
		{
			*tail = 0;	// 把\n 替换为‘终结符’
			fileMd5 = pre + 1;
			//cout << "hash: [" << fileMd5 << "] size: " << fileMd5.size() << endl;
		}
		_pclose(execfd);
	}

	return true;
}

bool IsFileHashEqual(string& path, string& hash)
{
	string curHash;

	if (!GetFileMd5(path, curHash))
	{
		cout << "GetFileMd5 failed" << endl;
		return false;
	}

	return curHash == hash;
}


void CheckPathExists(const string& curPath)
{
	int tail = curPath.find_last_of('/');
	int pos = 0;

	if (-1 == tail)
	{
		return;
	}

	string pureDir = curPath.substr(0, tail);
	string tpDir;

	while (tpDir != pureDir)
	{
		pos += curPath.substr(pos+1, tail).find_first_of('/')+1;
		tpDir = pureDir.substr(0, pos);
		//cout << "tpDir: " << tpDir << endl;

		// 如果该目录不存在
		if (0 != _access(tpDir.c_str(), 0))
		{
			//printf("_makedir [%s]\n", tpDir.c_str());
			_mkdir(tpDir.c_str());
		}
	}
}


typedef bool(*ParseFunCallback)(const char*, AppConfig *);


bool LoadConfig(const char* ConfigFilePath, ParseFunCallback ParseFunc, AppConfig *GS_acfg)
{
	FILE *fp = NULL;
	char  buf[MAXLINE];
	bool  ret = true;

	if ((fp = fopen(ConfigFilePath, "r")) == NULL)
	{
		perror(ConfigFilePath);
		return false;
	}

	while (!feof(fp))
	{
		memset(buf, 0, MAXLINE);
		fgets(buf, MAXLINE, fp);

		if (!ParseFunc(buf, GS_acfg))
		{
			ret = false;
			break;
		}
		//printf("key: %s value: %s\n", key, value);
	}

	fclose(fp);
	return ret;
}


// this is a callback function
// parse config file's line one by one
bool MyParseFunc(const char* buf, AppConfig* GS_acfg)
{
	static  const char *ConfigItems[] = {
		"SERVER_ADDR",
		"SERVER_PORT",
		"UPDATE_ADDR",
		"LOCAL_PORT",
	};


	char	key[MAXLINE];
	char	value[MAXLINE / 2];
	int 	strslen = 0;


	if (NULL == buf)
	{
		printf("buf is NULL\n");
		return false;
	}

	strslen = strlen(buf);
	// if gets blank or invalid length 
	// line just skip it
	if (strslen < 2 || strslen >= MAXLINE)
	{
		return true;
	}

	memset(key, 0, sizeof(key));
	memset(value, 0, sizeof(value));

	sscanf(buf, "%s %s\n", key, value);

	if (strlen(value) < 0)
	{
		return true;
	}
	if (!strcmp(key, ConfigItems[0]))
	{
		strcpy(GS_acfg->ServAddr, value);
	}
	else if (!strcmp(key, ConfigItems[1]))
	{
		GS_acfg->ServPort = atoi(value);
	}
	else if (!strcmp(key, ConfigItems[2]))
	{
		strcpy(GS_acfg->UpdateServ, value);
	}
	else if (!strcmp(key, ConfigItems[3]))
	{
		GS_acfg->LocalPort = atoi(value);
	}

	return true;
}


bool LoadConfig()
{
	string url;

	if (!LoadConfig(CONFIG_FILE, MyParseFunc, &GlobalConfig.aconfig))
	{
		return false;
	}

	url = "ftp://";
	url += GlobalConfig.aconfig.UpdateServ;
	url += "/CloudMonitor/";

	strncpy(GlobalConfig.update_url, url.c_str(), MAXLINE);
	cout << "update url: " << GlobalConfig.update_url << endl;

	return true;
}

int WriteToLog(char* str)
{
	static char LogFile[] = "Service.txt";
	static char timeBuf[MAX_PATH];

	// 如果没有开启‘日志功能’，则直接返回
	if (!GlobalConfig.enableLog)
	{
		return 0;
	}

	time_t timep;
	struct tm *p;


	time(&timep);
	p = localtime(&timep);
	memset(timeBuf, 0, sizeof(timeBuf));
	snprintf(timeBuf, MAX_PATH, "[%d-%02d-%02d %02d:%02d:%02d] ", \
		1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);


	FILE* log;
	fopen_s(&log, LogFile, "a+");

	if (log == NULL)
		return -1;

	fprintf(log, "%s%s\n", timeBuf, str);
	fclose(log);
	return 0;
}

void EnableLog()
{
	GlobalConfig.enableLog = true;
}


bool MyCreateProcess(LPCSTR appName, LPSTR appArgs)
{
	STARTUPINFOA   StartupInfo;		//创建进程所需的信息结构变量    
	PROCESS_INFORMATION pi;
	char output[MAXBYTE];

	if (NULL == appName)
	{
		return false;
	}

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	snprintf(output, sizeof(output), "%s", appName);

	if (CreateProcessA(NULL,
		output,
		NULL,
		NULL,
		FALSE,
		//0,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&StartupInfo,
		&pi))
	{

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		snprintf(output, MAXBYTE, "[ERROR] CreateProcess: %s", appName);
		return false;
	}

	return true;
}
