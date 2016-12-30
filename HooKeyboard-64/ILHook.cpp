#include "ILHook.h"
#include <stdio.h>

CILHook::CILHook()
{
	m_pfnOrig = NULL;
	ZeroMemory(m_bOldBytes, 5);
	ZeroMemory(m_bNewBytes, 5);
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
		ReadProcessMemory(GetCurrentProcess(), m_pfnOrig, m_bOldBytes, 5, &dwNum);

		m_bNewBytes[0] = '\xe9';
		*(SIZE_T*)(m_bNewBytes + 1) = (SIZE_T)pfnHookFunc - (SIZE_T)m_pfnOrig - 5;

		WriteProcessMemory(GetCurrentProcess(), m_pfnOrig, m_bNewBytes, 5, &dwNum);

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
		WriteProcessMemory(GetCurrentProcess(), m_pfnOrig, m_bOldBytes, 5, &dwNum);
	}
}

BOOL CILHook::ReHook()
{
	BOOL bRet = FALSE;

	if (m_pfnOrig != 0)
	{
		SIZE_T dwNum = 0;
		WriteProcessMemory(GetCurrentProcess(), m_pfnOrig, m_bNewBytes, 5, &dwNum);
		bRet = TRUE;
	}
	return bRet;
}
