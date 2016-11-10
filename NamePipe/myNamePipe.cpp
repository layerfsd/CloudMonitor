#include "iostream"
#include "windows.h"

using namespace std;

void   main(int argc, char* argv[])
{
	LPCTSTR Message = "the pipe's message from a client to server.";
	if (argc == 2)
		Message = argv[1];
	DWORD WriteNum;

	if (WaitNamedPipe("\\\\.\\Pipe\\Test", NMPWAIT_WAIT_FOREVER) == FALSE) {
		cout << "�ȴ�����ʧ�ܣ�" << endl;
		return;
	}

	HANDLE hPipe = CreateFile("\\\\.\\Pipe\\Test", GENERIC_READ | \
		GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		cout << "�ܵ���ʧ�ܣ�" << endl;
		return;
	}

	cout << "�ܵ����ӳɹ�" << endl;
	if (WriteFile(hPipe, Message, strlen(Message), &WriteNum, NULL) == FALSE) {
		cout << "����д��ܵ�ʧ�ܣ�" << endl;
	}
	CloseHandle(hPipe);
}