
#include <Windows.h>
#include <WtsApi32.h>

// 以当前用户身份启动一个进程
bool StartInteractiveProcess(LPSTR cmd, LPSTR cmdDir)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.lpDesktop = TEXT("winsta0\\default");  // Use the default desktop for GUIs
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	HANDLE token;
	DWORD sessionId = ::WTSGetActiveConsoleSessionId();
	if (sessionId == 0xffffffff)  // Noone is logged-in
		return false;
	// This only works if the current user is the system-account (we are probably a Windows-Service)
	HANDLE dummy;
	if (::WTSQueryUserToken(sessionId, &dummy)) {
		if (!::DuplicateTokenEx(dummy, TOKEN_ALL_ACCESS, NULL, SecurityDelegation, TokenPrimary, &token)) {
			::CloseHandle(dummy);
			return false;
		}
		::CloseHandle(dummy);
		// Create process for user with desktop
		if (!::CreateProcessAsUser(token, NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, cmdDir, &si, &pi)) {  // The "new console" is necessary. Otherwise the process can hang our main process
			::CloseHandle(token);
			return false;
		}
		::CloseHandle(token);
	}
	// Create process for current user
	else if (!::CreateProcess(NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, cmdDir, &si, &pi))  // The "new console" is necessary. Otherwise the process can hang our main process
		return false;
	// The following commented lines can be used to wait for the process to exit and terminate it
	//::WaitForSingleObject(pi.hProcess, INFINITE);
	//::TerminateProcess(pi.hProcess, 0);
	::CloseHandle(pi.hProcess);
	::CloseHandle(pi.hThread);
	return true;
}