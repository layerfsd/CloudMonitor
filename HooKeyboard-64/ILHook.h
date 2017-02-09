#pragma once
#ifndef __ILHOOK_H_INCLUDE_
#define __ILHOOK_H_INCLUDE_


#include <Windows.h>

#if _WIN64
#define INSTRUCTION_LEN		12
#else
#define INSTRUCTION_LEN		5
#endif

class CILHook
{
public:
	CILHook();
	~CILHook();

	BOOL Hook(LPSTR pszModuleName, LPSTR pszFuncName, PROC pfnHookFunc);

	VOID UnHook();
	BOOL ReHook();

private:
	PROC m_pfnOrig;
	BYTE m_bOldBytes[INSTRUCTION_LEN];
	BYTE m_bNewBytes[INSTRUCTION_LEN];
};

#endif