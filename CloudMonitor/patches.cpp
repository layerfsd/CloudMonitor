
// Use signal to attach a signal handler to the abort routine
#include "FileMon.h"
#include "network.h"
#include "patches.h"
#include "AutoStart.h"
#include "process.h"

#include <signal.h>
#include <tchar.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <io.h>
#include <tlhelp32.h>


#define THIS_APP_NAME			"CloudMonitor.exe"
#define DEPEND_APP_NAME			"MonitorService.exe"
#define DEPEND_APP_NAME_64		"MonitorService-64.exe"

#define BACKEND_FLAG			"--backend"


using namespace std;

extern BOOL g_RUNNING;


bool InformUser(int info);

bool GetMyName(char* szBuf, size_t bufSize)
{

	CHAR    szPath[MAX_PATH] = { 0 };

	if (!GetModuleFileNameA(NULL, szPath, MAX_PATH))
	{
		printf("GetModuleFileName failed (%d)\n", GetLastError());
		return false;
	}

	char* pos = NULL;

	pos = strrchr(szPath, '\\');

	if (NULL == pos)
	{
		return false;
	}

	strncpy(szBuf, pos + 1, bufSize);
	return true;
}



bool TryStartUp(const char* sem_name)
{
	printf("sem_name: %s\n", sem_name);

	HANDLE  semhd = OpenSemaphoreA(SEMAPHORE_MODIFY_STATE, FALSE, sem_name);

	// �򿪳ɹ���˵���Ѿ���ʵ��������
	if (NULL != semhd)
	{
		printf("%s is already running.\n", sem_name);
		return false;
	}
	// ��ʧ�ܣ���˵���������������
	// �����ź���
	if (NULL == CreateSemaphoreA(NULL, 1, 1, sem_name))
	{
		printf("Create [%s] failed.\n", sem_name);
		return false;
	}

	return true;
}


void SignalHandler(int signal)
{
	//system("cls");
	printf("\nCloudMonitor Exciting...\n");
	g_RUNNING = FALSE;
	//exit(signal);
	return;
}


void RegSigint()
{
	typedef void(*SignalHandlerPointer)(int);

	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGINT, SignalHandler);

	return;
}

BOOL IsDirectory(const char *pDir)
{
	char szCurPath[500];
	ZeroMemory(szCurPath, 500);
	sprintf_s(szCurPath, 500, "%s//*", pDir);
	WIN32_FIND_DATAA FindFileData;
	ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATAA));

	HANDLE hFile = FindFirstFileA(szCurPath, &FindFileData); /**< find first file by given path. */

	if (hFile == INVALID_HANDLE_VALUE)
	{
		FindClose(hFile);
		return FALSE; /** ��������ҵ���һ���ļ�����ôû��Ŀ¼ */
	}
	else
	{
		FindClose(hFile);
		return TRUE;
	}

}



bool MyCreateProcess(LPCSTR appName, LPSTR appArgs = NULL)
{
	STARTUPINFOA   StartupInfo;		//���������������Ϣ�ṹ����    
	PROCESS_INFORMATION pi;
	char output[MAXBYTE];
	char cmd[MAXBYTE];

	if (NULL == appName)
	{
		return false;
	}

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	snprintf(cmd, sizeof(output), "%s %s", appName, appArgs);

	snprintf(output, sizeof(output), "[CloudMonitor-->CreateProcess]%s %s", appName, appArgs);

	WriteToLog(output);

	if (CreateProcessA(NULL,
		cmd,
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
		WriteToLog(output);
		return false;
	}

	return true;

}


typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL IsWow64()
{
	BOOL bIsWow64 = FALSE;

	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.

	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			//handle error
		}
	}
	return bIsWow64;
}


bool StartHookService()
{

	DWORD	dwPid;

	// �Ҳ���Ŀ�����ʱ��������
	if (!FindProcessPid(DEPEND_APP_NAME, dwPid))
	{
		MyCreateProcess(DEPEND_APP_NAME, BACKEND_FLAG);
		WriteToLog("[CloudMonitor starting] " DEPEND_APP_NAME);
	}
	
	// ���ϵͳ�Ƿ�֧��64λ��������
	if (IsWow64() && !FindProcessPid(DEPEND_APP_NAME_64, dwPid))
	{
		MyCreateProcess(DEPEND_APP_NAME_64, BACKEND_FLAG);
		WriteToLog("[CloudMonitor starting] " DEPEND_APP_NAME_64);
	}
	return true;
}

void SetWorkPath()
{
	char strModule[MAX_PATH];
	GetModuleFileName(NULL, strModule, MAX_PATH); //�õ���ǰģ��·��
	strcat(strModule, "\\..\\");     //����Ϊ��ǰ����·��Ϊ��ʱ����һ��
	SetCurrentDirectory(strModule);
}

void InitDir(bool hide)
{
	char	sem_name[MAX_PATH];
	// ������־�ļ�����ȡ�������� YYYY-MM-DD.txt
	char LogName[MAX_PATH];
	FILE *stream;
	time_t timep;
	struct tm *p;


	memset(sem_name, 0, MAX_PATH);

	time(&timep);
	p = localtime(&timep);
	memset(LogName, 0, sizeof(LogName));
	snprintf(LogName, MAX_PATH, "LOG\\%d-%d-%d[Client].txt", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday);

	// ���ؿ���̨����
	if (hide)
	{
		ShowWindow(GetConsoleWindow(), SW_HIDE);
		if ((stream = freopen(LogName, "a+", stdout)) == NULL)
		{
			exit(-1);
		}
	}

	memset(LogName, 0, sizeof(LogName));
	snprintf(LogName, MAX_PATH, "LOG\\%d-%d-%d %02d:%02d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min);
	printf("\n\n\n\n\n[Start Time] %s\n", LogName);


	GetMyName(sem_name, MAX_PATH);
	
	if (!TryStartUp(sem_name))
	{
		InformUser(ALREADY_LOGIN);
		exit(3);
	}

	RegSigint(); //ע�� CTRL+C �źŴ���,������ֹ�Ự.

	//StartHookService();

	// ��鱾�����Ƿ��Ѿ����뵽������������Ŀ
	// RegisterProgram();

	// ������ʱĿ¼
	const char *dirs[] = {
		"DATA", "LOG", "TMP",
	};

	for (int i = 0; i < ArraySize(dirs); i++)
	{
		if (-1 == _access(dirs[i], 0))
		{
			cout << TMP_DIR << " not exists!!!" << endl;
			cout << "mkdir: " << TMP_DIR << endl;
			_mkdir(dirs[i]);
		}
	}
	system("del /F /S /Q TMP"); 	// ����ÿ������ʱ,�����ʱĿ¼

	return;
}





bool InformUser(int info)
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(159);
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("[INFORM] connect error !");
		closesocket(sclient);
		return 0;
	}

	char Buf[4] = { 0, 0, 0, 0 };

	sprintf(Buf, "%d", info);
	
	if (!send(sclient, Buf, strlen(Buf), 0))
	{
		cout << "д������ʧ�� ..." << endl << endl;
		return false;
	}
	cout << "д�����ݳɹ���    " << info << endl << endl;
	closesocket(sclient);
	WSACleanup();

	return true;
}



int WriteToLog(char* str)
{
	static char LogFile[] = "Service.txt";
	static char timeBuf[MAX_PATH];


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

void CleanTmpFiles(SFile& file)
{
	remove(file.savedPath.c_str());		// tmp\doc
	remove(file.encPath.c_str());		// tmp\aes
	remove(file.txtPath.c_str());		// tmp\txt
}
