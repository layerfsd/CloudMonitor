#include <Windows.h>
#include <iostream>

using namespace std;
//���������ڿͻ���ͨ�� CreateFile �򿪵������ܵ����


const char * pStr = "CloudMonitor";
const char * pPipeName = "\\\\.\\pipe\\CloudMonitor";
//�������ܵ�
bool OpenNamedPipeInClient(HANDLE& hNamedPipe);


//�ͻ����������ܵ���д������
bool NamedPipeWriteInClient(HANDLE& hNamedPipe, const char *buf);
#include <stdio.h>

int main(int argc, char *argv[])
{
	//char tmp[MAX_PATH] = "F:\\NutStore\\SSL����\\��ȫ�칫��Ϣ���ƽ̨��Ŀ�з�����20160922.docx";
	char path[MAX_PATH];
	char cmd[MAX_PATH] = "FileMon.exe wps";
	int proPid = 8964;

	HANDLE            hNamedPipe;
	OpenNamedPipeInClient(hNamedPipe);


	FILE   *pPipe;

	snprintf(cmd, MAX_PATH, "FileMon.exe %d",  proPid);
	//fputs(cmd, stdout);
	if ((pPipe = _popen(cmd, "r")) == NULL)
	{
		return -1;
	}

	while (true)
	{
		memset(path, 0, sizeof(path));
		if (!fgets(path, MAX_PATH, pPipe))
		{
			printf(path);
		}
	}

	_pclose(pPipe);

		//�������ܵ���д������
		//NamedPipeWriteInClient(hNamedPipe, tmp);

	return 0;
}


bool OpenNamedPipeInClient(HANDLE& hNamedPipe)
{
	//�ȴ����������ܵ�
	if (!WaitNamedPipe(pPipeName, NMPWAIT_WAIT_FOREVER))
	{
		cout << "�����ܵ�ʵ�������� ..." << endl << endl;
		return false;
	}
	//�������ܵ�
	hNamedPipe = CreateFile(pPipeName, GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hNamedPipe)
	{
		cout << "�������ܵ�ʧ�� ..." << endl << endl;
		return false;
	}

	return true;
}


bool NamedPipeWriteInClient(HANDLE& hNamedPipe, const char *buf)
{
	DWORD                dwWrite;
	//�������ܵ���д������
	if (!WriteFile(hNamedPipe, buf, strlen(buf), &dwWrite, NULL))
	{
		cout << "д������ʧ�� ..." << endl << endl;
		return false;
	}

	return true;
}
