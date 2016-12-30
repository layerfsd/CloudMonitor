// ViewStartUp.cpp : 定义控制台应用程序的入口点。
//

#include "AutoStart.h"
#include <iostream>
using std::cout; using std::endl;

BOOL IsMyProgramRegisteredForStartup(char* pszAppName)
{
	HKEY hKey = NULL;
	LONG lResult = 0;
	BOOL fSuccess = TRUE;
	DWORD dwRegType = REG_SZ;
	char szPathToExe[MAX_PATH] = {};
	DWORD dwSize = sizeof(szPathToExe);

	lResult = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);

	fSuccess = (lResult == 0);

	if (fSuccess)
	{
		lResult = RegQueryValueExA(hKey, pszAppName, NULL, &dwRegType, (LPBYTE)szPathToExe, &dwSize);
		fSuccess = (lResult == 0);
	}

	if (fSuccess)
	{
		fSuccess = (strlen(szPathToExe) > 0) ? TRUE : FALSE;
	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}

BOOL RegisterMyProgramForStartup(PCSTR pszAppName, PCSTR pathToExe, PCSTR args)
{
	HKEY hKey = NULL;
	LONG lResult = 0;
	BOOL fSuccess = TRUE;
	DWORD dwSize;

	const size_t count = MAX_PATH * 2;
	char  szValue[count] = {};

	// fix path contains spaces
	snprintf(szValue, count, "\"%s\" ", pathToExe);

	if (args != NULL)
	{
		// caller should make sure "args" is quoted if any single argument has a space
		// e.g. (L"-name \"Mark Voidale\"");
		strcat_s(szValue, count, args);
	}

	lResult = RegCreateKeyExA(HKEY_CURRENT_USER, STARTUP_PATH, 0, NULL, 0, (KEY_WRITE | KEY_READ), NULL, &hKey, NULL);

	fSuccess = (lResult == 0);

	if (fSuccess)
	{
		dwSize = (strlen(szValue) + 1) * 2;
		lResult = RegSetValueExA(hKey, pszAppName, 0, REG_SZ, (BYTE*)szValue, dwSize);
		fSuccess = (lResult == 0);
	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}


/*
eg:
fullPath: C:\User\helen\app.exe
buf:	  app.exe
*/
static bool GetNameFromPath(char* src, char* dst)
{
	if (NULL == src || NULL == dst)
	{
		return false;
	}
	char *pre = NULL;
	char *tail = NULL;

	tail = strrchr(src, '.');
	if (NULL == tail)
	{
		return false; //找不到文件后缀，则无法判断文件类型
	}


	pre = strrchr(src, '\\');
	if (NULL == pre)
	{
		pre = src;
		return true;
	}

	pre++;

	memset((void *)pre, 0, 50);
	strcpy(pre, "Daemon.exe");

	while (*pre != '.' && pre != tail)
	{
		*dst++ = *pre++;
	}
	*dst = 0;

	return true;
}

BOOL RegisterProgram()
{
	char szPathToExe[MAX_PATH] {};
	char szAppName[MAX_PATH] {};

	GetModuleFileNameA(NULL, szPathToExe, MAX_PATH);

	if (!GetNameFromPath(szPathToExe, szAppName))
	{
		return false;
	}


	// if already registed
	if (IsMyProgramRegisteredForStartup(szAppName))
	{
		return true;
	}

	printf("Add [%s]\n", szPathToExe);

	return RegisterMyProgramForStartup(szAppName, szPathToExe, STARTUP_ARGS);
}


void DoSomething()
{
	while (TRUE)
	{
		Sleep(1000);
	}
}

static uchar salt[64] = {
	0x96, 0x42, 0x2e, 0xd0, 0xf7, 0x3e, 0x8f, 0x70,
	0x6f, 0x7c, 0x84, 0x0b, 0x40, 0x07, 0xe6, 0x4f,
	0x92, 0x88, 0x74, 0xdc, 0x82, 0xe2, 0x02, 0x75,
	0x48, 0xdd, 0x47, 0xdc, 0x1d, 0x33, 0x79, 0x2b,

	0x53, 0x6b, 0xbe, 0x41, 0x49, 0x48, 0xcb, 0x61,
	0x5b, 0xc2, 0xeb, 0x71, 0x27, 0x36, 0x8c, 0xfe,
	0x26, 0x67, 0x7a, 0x25, 0xaa, 0xe2, 0x3b, 0x48,
	0xfe, 0x53, 0x88, 0xbc, 0x6e, 0xbe, 0x93, 0x85, };


inline void Xor(uchar* src, uchar* dst, int len)
{
	for (int i = 0; i < len; i++)
	{
		dst[i] = src[i] ^ salt[i];
	}
}

inline void ShowBytes(uchar* data, int len)
{
	for (int i = 0; i < len; i++)
	{
		printf("0x%02x ", data[i]);
		if ((i + 1) % 8 == 0)
		{
			printf("\n");
		}
	}
}


inline void ShowAccount(Account* act)
{
	printf("Username: %s\nPassword: %s\n", act->username, act->password);
}


HKEY OpenKey(HKEY hRootKey, char* strKey)
{
	HKEY hKey;
	LONG nError = RegOpenKeyExA(hRootKey, strKey, NULL, KEY_ALL_ACCESS, &hKey);

	if (nError == ERROR_FILE_NOT_FOUND)
	{
		cout << "Creating registry key: " << strKey << endl;
		nError = RegCreateKeyExA(hRootKey, strKey, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	}

	if (nError)
		cout << "Error: " << nError << " Could not find or create " << strKey << endl;

	return hKey;
}

inline bool SetVal(HKEY hKey, LPCSTR lpValue, uchar* data, DWORD size)
{
	LONG nError = RegSetValueExA(hKey, lpValue, NULL, REG_BINARY, (LPBYTE)data, size);

	if (nError)
	{
		cout << "Error: " << nError << " Could not set registry value: " << (char*)lpValue << endl;
		return false;
	}
	return true;
}

inline bool GetVal(HKEY hKey, LPCSTR lpValue, uchar* data, DWORD size)
{
	DWORD type = REG_BINARY;
	LONG nError = RegQueryValueExA(hKey, lpValue, NULL, &type, (LPBYTE)data, &size);

	if (nError == ERROR_FILE_NOT_FOUND)
		data = 0; // The value will be created and set to data next time SetVal() is called.
	else if (nError)
		cout << "Error: " << nError << " Could not get registry value " << (char*)lpValue << endl;

	return true;
}

bool GetAuth(Account* act)
{
	uchar data[64];

	memset(data, 0, sizeof(data));
	HKEY hKey = OpenKey(HKEY_CURRENT_USER, STORE_AUTH_PATH);
	GetVal(hKey, AUTH_NAME, data, 64);
	Xor(data, (uchar *)act, 64);
	return true;
}


bool SetAuth(Account* act)
{
	uchar data[64];

	memset(data, 0, sizeof(data));
	HKEY hKey = OpenKey(HKEY_CURRENT_USER, STORE_AUTH_PATH);
	
	// 加密账户
	Xor((uchar *)act, data, 64);
	SetVal(hKey, AUTH_NAME, data, 64);
	return true;
}
