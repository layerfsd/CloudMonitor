#include "ILHook.h"
#include <stdio.h>

CILHook::CILHook()
{
	m_pfnOrig = NULL;
	ZeroMemory(m_bOldBytes, sizeof(m_bOldBytes));
	ZeroMemory(m_bNewBytes, sizeof(m_bNewBytes));
}

CILHook::~CILHook()
{
	UnHook();
}

BOOL CILHook::Hook(LPSTR pszModuleName, LPSTR pszFuncName, PROC pfnHookFunc)
{
	BOOL bRet = FALSE;

	m_pfnOrig = (PROC)GetProcAddress(GetModuleHandle(pszModuleName), pszFuncName);

	if (m_pfnOrig != NULL)
	{
		SIZE_T dwNum = 0;
		ReadProcessMemory(GetCurrentProcess(), m_pfnOrig, m_bOldBytes, INSTRUCTION_LEN, &dwNum);

#if _WIN64
		m_bNewBytes[0] = '\x48';
		m_bNewBytes[1] = '\xb8';

		*(SIZE_T*)(m_bNewBytes + 2) = (SIZE_T)pfnHookFunc;

		m_bNewBytes[10] = '\x50';
		m_bNewBytes[11] = '\xc3';

#else
		m_bNewBytes[0] = '\xe9';
		*(SIZE_T*)(m_bNewBytes + 1) = (SIZE_T)pfnHookFunc - (SIZE_T)m_pfnOrig - INSTRUCTION_LEN;
#endif

		WriteProcessMemory(GetCurrentProcess(), m_pfnOrig, m_bNewBytes, INSTRUCTION_LEN, &dwNum);
		bRet = TRUE;
		printf("hook: %s OK\n", pszFuncName);
	}
	else
	{
		printf("hook: %s Failed\n", pszFuncName);
	}
	return bRet;
}

VOID CILHook::UnHook()
{
	if (m_pfnOrig != 0)
	{
		SIZE_T dwNum = 0;
		WriteProcessMemory(GetCurrentProcess(), m_pfnOrig, m_bOldBytes, INSTRUCTION_LEN, &dwNum);
	}
}

BOOL CILHook::ReHook()
{
	BOOL bRet = FALSE;

	if (m_pfnOrig != 0)
	{
		SIZE_T dwNum = 0;
		WriteProcessMemory(GetCurrentProcess(), m_pfnOrig, m_bNewBytes, INSTRUCTION_LEN, &dwNum);
		bRet = TRUE;
	}
	return bRet;
}
