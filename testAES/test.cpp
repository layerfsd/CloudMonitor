#include "encrept.h"



void test(char* strs, char* user_pass)
{
	char*	text;
	char    passwd[MAX_PASSWD_LEN] = "ILoveYou";
	char	reserved[] = "Hello Winter.";

	text = reserved;


	if (NULL != strs)
	{
		text = strs;
	}
	if (NULL != user_pass)
	{
		memcpy(passwd, user_pass, MAX_PASSWD_LEN);
	}
	

	int		textSize = strlen(text);

	size_t  enc_len = 0;
	size_t  dec_len = 0;

	char*	enc_buf = NULL;
	char*   dec_buf = NULL;

	printf("\n----------------->start test<----------------------------\n");

	if (!aes_cbc_256(text, textSize, passwd, &enc_buf, &enc_len, true))
	{
		printf("Encrept Error!");
		return;
	}

	if (!aes_cbc_256(enc_buf, enc_len, passwd, &dec_buf, &dec_len, false))
	{
		printf("Decrept Error!");
		return;
	}


	printf("Original:\n");
	printf("text: %s\n", text);
	hex_print(text, textSize);

	printf("Encrept:\n");
	hex_print(enc_buf, enc_len);

	printf("Decrept:\n");
	printf("text: %s\n", dec_buf);
	hex_print(dec_buf, dec_len);

	if (enc_buf) free(enc_buf);
	if (dec_buf) free(dec_buf);

	printf("\n----------------->start end<----------------------------\n\n");

	return;
}


void testFile()
{
	const char* filePath = "DATA\\test.docx";
	const char* dstPath  = "DATA\\test.docx.aes";
	const char* passwd   = "123456";

	EncreytFile(filePath, dstPath, passwd);
}


bool EncreytFile(const char* oriFileName, const char* encFileName, const char* passwd)
{
	char* CMD_FMT = "ssl\\openssl aes-256-cbc -in %s -out %s -pass pass:%s";
	char  cmd[_MAX_PATH];
	FILE* execfd = NULL;

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, CMD_FMT, oriFileName, encFileName, passwd);
	printf("cmd: [%s]\n", cmd);

	execfd = _popen(cmd, "r");

	if (NULL == execfd)
	{
		return false;
	}

	_pclose(execfd);
	return true;
}