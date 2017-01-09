#pragma once
#ifndef  _PATCHES_ALBERT__
#define _PATCHES_ALBERT__
#include <Windows.h>

#define ArraySize(ptr)	(sizeof(ptr) / sizeof(ptr[0]))
#define ErrorMsg(strs)  printf("ErrorMsg [%s] func [%s] line [%d]\n", strs, __func__, __LINE__);


void InitDir(bool hide);
bool StartHookService();	// ���� MonitorService.exe

void SetWorkPath();


// �ж��û�����ϵͳ�Ƿ�Ϊwin7
bool IsWin7();

//CopyFrom: https://msdn.microsoft.com/en-us/library/windows/desktop/ms684139(v=vs.85).aspx
BOOL IsWow64();		// ��⵱ǰϵͳ�Ƿ�֧��64λ��������

// ��¼������־
int WriteToLog(char* str);

// ɾ����ʱ�ļ�
void CleanTmpFiles(SFile& file);

#endif // ! _PATCHES_ALBERT__
