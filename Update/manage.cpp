#include "stdafx.h"

using namespace std;


// 以下三个进程，需要在重启更新程序运行时重新启动的
LPCSTR TargetProcessName[]{ "CloudMonitor.exe", "MonitorService.exe", "MonitorService-64.exe" };



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

int DownloadFtpFile(const char* url, FtpFile &ftpfile)
{
	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERPWD, FTP_AUTH);
		// Define our callback to get called when there's data to be written //  
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FetchFiles);
		// Set a pointer to our struct to pass to the callback //  
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);

		// Switch on full protocol/debug output //  
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);

		// always cleanup   
		curl_easy_cleanup(curl);

		if (CURLE_OK != res)
		{
			//we failed   
			fprintf(stderr, "curl told us %d\n", res);
		}
	}

	if (ftpfile.stream)
		fclose(ftpfile.stream); // close the local file   

	curl_global_cleanup();

	return 0;
}
//end: http://blog.csdn.net/exlsunshine/article/details/29177025