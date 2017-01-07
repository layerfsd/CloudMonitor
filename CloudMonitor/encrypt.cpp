#include "Encrypt.h"
#include <Windows.h>

void testFile()
{
	const char* filePath = "DATA\\test.docx";
	const char* dstPath = "DATA\\test.docx.aes";
	const char* passwd = "123456";

	EncreptFile(filePath, dstPath, passwd);
}


bool EncreptFile(const char* oriFileName, const char* encFileName, const char* passwd)
{
	char* CMD_FMT = "tools\\7z a -p%s \"%s\"  \"%s\" ";	//防止文件路径中包含空格
	char  cmd[_MAX_PATH];
	FILE* execfd = NULL;

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, CMD_FMT, passwd, encFileName, oriFileName);
	printf("cmd: [%s]\n", cmd);

	execfd = _popen(cmd, "r");

	if (NULL == execfd)
	{
		return false;
	}

	return (0 == _pclose(execfd));
}