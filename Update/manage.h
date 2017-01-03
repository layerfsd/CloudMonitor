#pragma once

#include <Windows.h>	// for VOID declaration

VOID StopMyService();
BOOL SendSignal2Process(DWORD dwPid, DWORD dwSig);
BOOL FindProcessPid(LPCSTR ProcessName, DWORD& dwPid);
VOID SendControlC(DWORD pid);


bool GetFilesList(char* tempFile);