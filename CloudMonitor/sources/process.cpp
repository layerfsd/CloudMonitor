#include "process.h"
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

struct monProcessFmt
{
	int		code;
	char*	name;
};


static monProcessFmt monProcessList[] = {
	{ 101, "QQ.exe" },
	{ 103, "iexplore.exe" },
	{ 104, "QQBrowser.exe" },
	{ 105, "360se.exe" },
	{ 107, "firefox.exe" },
	{ 108, "chrome.exe" },
	{ 109, "sogouexplorer.exe" },
	{ 110, "opera.exe" },
	{ 111, "Maxthon.exe" },
	{ 112, "通用浏览器" },
	//{ 201, "wps.exe" }
};	  

static int monProcessNum = sizeof(monProcessList) / sizeof(monProcessList[0]);

//int main(void)
//{
//	GetProcessList();
//	return 0;
//}

BOOL ListProcessModules(DWORD dwPID);
BOOL ListProcessThreads(DWORD dwOwnerPID);
void printError(TCHAR* msg);


bool ShowProcessList(vector<Process>& plst)
{
	if (plst.size() <= 0)
		return FALSE;

	printf("\nseq %-20s %-5s status isShutdown\n", "ProcessName", "pid");
	for (DWORD i = 0; i < plst.size(); i++)
	{
		printf("%-3d %-20s %-5d   %d	%d\n", plst[i].seq, plst[i].name, plst[i].pid, plst[i].status, plst[i].shutdown);
	}
	return true;
}


int  KillProcess(vector<Process>& plst, bool KillAll)
{
	if (plst.size() <= 0)
	{
		return 0;
	}

	int cnt = 0;
	HANDLE hnh = NULL;

	for (size_t i = 0; i < plst.size(); i++)
	{
		if (KillAll || plst[i].shutdown)
		{
			hnh = OpenProcess(PROCESS_ALL_ACCESS, FALSE, plst[i].pid);
			if (NULL != hnh)
			{
				if (TerminateProcess(hnh, 0))
				{
					cnt += 1;
					plst[i].status = CLOSED;
				}
				else
				{
					printError(plst[i].name);
				}
				CloseHandle(hnh);
			}
			else
			{
				printError(plst[i].name);
			}
		}// end CMD_SHUTDOWN
	}// end for

	return cnt;
}


bool GenKillResult(vector<Process>& plst, string& message)
{
	char buf[8];
	bool ret = false;


	if (plst.size() <= 0)
	{
		cout << "Empty Killing List ..." << endl;
		return ret;
	}

	for (size_t i = 0; i < plst.size(); i++)
	{
		if (CLOSED == plst[i].status)
		{
			ret = false;
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%d ", plst[i].seq);
			message += buf;
		}
	}

	return ret;
}

static vector<Process> LocalProcesslist;

bool CheckNetworkApps(vector<Process>& plst, string& logMsg)
{
	// 没有获取到常用进程,返回
	if (!GetProcessList(plst))
	{
		return false;
	}

	//char *LogPrefix = "用户在运行网络程序: ";
	char tmpBuf[MAX_PATH];

	logMsg.clear();
	logMsg = FILE_NETEDIT;		// 设置警报类型
	logMsg += ' ';
	for (DWORD i = 0; i < plst.size()-1; i++)
	{
		memset(tmpBuf, 0, sizeof(tmpBuf));
		sprintf(tmpBuf, "%d,", plst[i].code);
		logMsg += tmpBuf;
	}

	sprintf(tmpBuf, "%d", plst[plst.size()-1].code);
	logMsg += tmpBuf;

	return true;
}

bool RemoteGetProcessList(string& message, string& args)
{
	char tBuf[256];;

	if (!GetProcessList(LocalProcesslist))
	{
		message = "No Monited Process Running";
		return false;
	}

	memset(tBuf, 0, sizeof(tBuf));
	sprintf(tBuf, "%d\n", LocalProcesslist.size());		//记录进程总个数
	message = tBuf;

	for (size_t i = 0; i < LocalProcesslist.size(); i++)
	{
		memset(tBuf, 0, sizeof(tBuf));
		sprintf(tBuf, "%d-%d\n", LocalProcesslist[i].seq, LocalProcesslist[i].code);
		message += tBuf;
	}
	return true;
}


bool parseProcessSeq(string& args, vector<int>& seqList)
{
	int		count = 0;
	char*		largs = NULL;
	char*		token = NULL;

	largs = new char[args.length() + 1];

	if (NULL == largs)
	{
		cout << "failed new char " << args.length() << endl;
		return false;
	}
	strcpy(largs, args.c_str());


	token = strtok(largs, "\n");
	count = atoi(token);	//获取要终结进程的总数
	cout << "Count: " << count << endl;


	if (NULL == largs)
	{
		cout << "failed new int " << count << endl;
		return false;
	}

	int			pos = 0;
	int			tp = 0;
	while (NULL != token && pos < count)
	{
		token = strtok(NULL, "\n");

		tp = atoi(token);
		seqList.push_back(tp);
		//cout << "pos: " << pos << " seq: " << seqList[pos] << endl;
		pos += 1;
	}


	delete[] largs;
	return true;
}


// 通过Pid杀死一个进程
inline bool KillProcessByPid(DWORD pid)
{
	HANDLE hnh = NULL;
	bool   ret = true;

	hnh = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (NULL != hnh)
	{
		if (TerminateProcess(hnh, 0))
		{
			ret = true;
		}
		else
		{
			ret = false;
		}
		CloseHandle(hnh);
	}
	else // 获取进程信息失败,说明该进程已经被杀掉,所以返回true
	{
		ret = true;
	}

	return ret;
}


 bool rKillProcess(vector<Process>& plst, vector<int>& seqList, string& logMsg)
 {
	 char   num[16];
	 bool   allDone = true;	//是否所有任务都完成
	 DWORD  pid = 0;
	 int    seq = 0;

	 logMsg.clear();
	 cout << "Server ask me to kill: [" << seqList.size() << "] process" << endl;

	 for (size_t j = 0; j < seqList.size(); j++)
	 {
		 seq = seqList[j];
		 pid = plst[seq].pid;  // 根据进程序号定位到到进程 Pid
		 cout << "killing " << seq+1 << endl;
	
	 	 memset(num, 0, sizeof(num));
		 
#if 0
	 	 if (KillProcessByPid(plst[j].pid))	   //判断是否成功关闭指定Pid 的进程
	 	 {
	 	 	sprintf(num, "%d\n", seq);
	 	 }
	 	 else
	 	 {
	 	 	sprintf(num, "-%d\n", seq);
	 	 	allDone = false;
	 	 }
#else
		 KillProcessByPid(plst[j].pid);

#endif
		 sprintf(num, "%d\n", seq);
		 logMsg += num;						//记录关闭进程的序列号
		 //printf("j: %d seq: %d\n", j, seqList[j]);
	 }//end for

	 return allDone;
 }



 //设置回复消息(message)
//		成功:OK
//		失败:FAILED
bool RemoteKillProcess(string& message, string& args)
{
	vector<int> seqList;
	bool		allDone = true;

	if (parseProcessSeq(args, seqList))
	{
		allDone = rKillProcess(LocalProcesslist, seqList, message);
	}
	else
	{
		allDone = false;
		message = "Failed to parseProcessSeq()";
	}

	return allDone;
}

BOOL GetProcessList(vector<Process>& plst)
{
	HANDLE hProcessSnap;
//	HANDLE hProcess;
	PROCESSENTRY32 pe32;
//	DWORD dwPriorityClass;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		printError(TEXT("Process32First")); // show cause of failure
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	Process pro;
	bool	NoEmpty = false;
	do
	{
		//_tprintf(TEXT("\n\n====================================================="));
		//_tprintf(TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile);
		//_tprintf(TEXT("\n-------------------------------------------------------"));

		memset(&pro, 0, sizeof(pro));
		strncpy(pro.name, pe32.szExeFile, MAX_PATH);
		pro.pid = pe32.th32ProcessID;
		pro.status = RUNNING;
		pro.shutdown = false;

		for (int i = 0; i < monProcessNum; i++)
		{
			if (!_strnicmp(pe32.szExeFile, monProcessList[i].name, MAX_PATH))
			{
				NoEmpty = true;
				//_tprintf(TEXT("NAME:  %-20s PID: %-5d\n"), pe32.szExeFile, pe32.th32ProcessID);
				pro.seq = plst.size();
				pro.code = monProcessList[i].code;
				plst.push_back(pro);
				break;
			}
		}
#if 0
		// Retrieve the priority class.
		dwPriorityClass = 0;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if (hProcess == NULL)
			printError(TEXT("OpenProcess"));
		else
		{
			dwPriorityClass = GetPriorityClass(hProcess);
			if (!dwPriorityClass)
				printError(TEXT("GetPriorityClass"));
			CloseHandle(hProcess);
		}

		_tprintf(TEXT("\n  Process ID        = 0x%08X"), pe32.th32ProcessID);
		_tprintf(TEXT("\n  Thread count      = %d"), pe32.cntThreads);
		_tprintf(TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID);
		_tprintf(TEXT("\n  Priority base     = %d"), pe32.pcPriClassBase);
		if (dwPriorityClass)
			_tprintf(TEXT("\n  Priority class    = %d"), dwPriorityClass);

		// List the modules and threads associated with this process
		ListProcessModules(pe32.th32ProcessID);
		ListProcessThreads(pe32.th32ProcessID);
#endif
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return NoEmpty;
}


BOOL ListProcessModules(DWORD dwPID)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		printError(TEXT("CreateToolhelp32Snapshot (of modules)"));
		return(FALSE);
	}

	// Set the size of the structure before using it.
	me32.dwSize = sizeof(MODULEENTRY32);

	// Retrieve information about the first module,
	// and exit if unsuccessful
	if (!Module32First(hModuleSnap, &me32))
	{
		printError(TEXT("Module32First"));  // show cause of failure
		CloseHandle(hModuleSnap);           // clean the snapshot object
		return(FALSE);
	}

	// Now walk the module list of the process,
	// and display information about each module
	do
	{
		_tprintf(TEXT("\n\n     MODULE NAME:     %s"), me32.szModule);
		_tprintf(TEXT("\n     Executable     = %s"), me32.szExePath);
		_tprintf(TEXT("\n     Process ID     = 0x%08X"), me32.th32ProcessID);
		_tprintf(TEXT("\n     Ref count (g)  = 0x%04X"), me32.GlblcntUsage);
		_tprintf(TEXT("\n     Ref count (p)  = 0x%04X"), me32.ProccntUsage);
		_tprintf(TEXT("\n     Base address   = 0x%08X"), (DWORD)me32.modBaseAddr);
		_tprintf(TEXT("\n     Base size      = %d"), me32.modBaseSize);

	} while (Module32Next(hModuleSnap, &me32));

	CloseHandle(hModuleSnap);
	return(TRUE);
}

BOOL ListProcessThreads(DWORD dwOwnerPID)
{
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;

	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32);

	// Retrieve information about the first thread,
	// and exit if unsuccessful
	if (!Thread32First(hThreadSnap, &te32))
	{
		printError(TEXT("Thread32First")); // show cause of failure
		CloseHandle(hThreadSnap);          // clean the snapshot object
		return(FALSE);
	}

	// Now walk the thread list of the system,
	// and display information about each thread
	// associated with the specified process
	do
	{
		if (te32.th32OwnerProcessID == dwOwnerPID)
		{
			_tprintf(TEXT("\n\n     THREAD ID      = 0x%08X"), te32.th32ThreadID);
			_tprintf(TEXT("\n     Base priority  = %d"), te32.tpBasePri);
			_tprintf(TEXT("\n     Delta priority = %d"), te32.tpDeltaPri);
			_tprintf(TEXT("\n"));
		}
	} while (Thread32Next(hThreadSnap, &te32));

	CloseHandle(hThreadSnap);
	return(TRUE);
}

void printError(TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));

	// Display the message
	_tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}
