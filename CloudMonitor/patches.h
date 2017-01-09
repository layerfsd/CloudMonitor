#pragma once
#ifndef  _PATCHES_ALBERT__
#define _PATCHES_ALBERT__
#include <Windows.h>


#define SERVICE_NAME			"CloudMonitorService"

#define ArraySize(ptr)	(sizeof(ptr) / sizeof(ptr[0]))
#define ErrorMsg(strs)  printf("ErrorMsg [%s] func [%s] line [%d]\n", strs, __func__, __LINE__);

#define UPDATE_CHECKED_FLAG	"UPDATE_CHECKED"


void InitDir(bool hide);
bool StartHookService();	// ���� MonitorService.exe

void SetWorkPath();


int GetServiceStatus(const char* name);

bool IsServiceRunning();

// �ж��û�����ϵͳ�Ƿ�Ϊwin7
bool IsWin7();

//CopyFrom: https://msdn.microsoft.com/en-us/library/windows/desktop/ms684139(v=vs.85).aspx
BOOL IsWow64();		// ��⵱ǰϵͳ�Ƿ�֧��64λ��������

// ��¼������־
int WriteToLog(char* str);

// ɾ����ʱ�ļ�
void CleanTmpFiles(SFile& file);


VOID __stdcall DoStartSvc(const char* szSvcName);

#endif // ! _PATCHES_ALBERT__
