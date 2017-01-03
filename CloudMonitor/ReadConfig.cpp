#include "ReadConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool LoadConfig(const char* ConfigFilePath, ParseFunCallback ParseFuncCallback, AppConfig *GS_acfg)
{
	FILE *fp = NULL;
	char  buf[MAXLINE];
	bool  ret = true;

	if ((fp = fopen(ConfigFilePath, "r")) == NULL)
	{
		perror(ConfigFilePath);
		return false;
	}
	
	while (!feof(fp))
	{
		memset(buf, 0, MAXLINE);
		fgets(buf, MAXLINE, fp);

		if (!ParseFuncCallback(buf, GS_acfg))
		{
			ret = false;
			break;
		}
		//printf("key: %s value: %s\n", key, value);
	}

	fclose(fp);
	return ret;
}


// this is a callback function
// parse config file's line one by one
bool MyParseFunc(const char* buf, AppConfig* GS_acfg)
{
	static  const char *ConfigItems[] = {
		"SERVER_ADDR", 
		"SERVER_PORT",
		"UPDATE_ADDR",
		"LOCAL_PORT",
	};


	char	key[MAXLINE];
	char	value[MAXLINE/2];
	int 	strslen = 0;


	if (NULL == buf)
	{
		printf("buf is NULL\n");
		return false;
	}

	strslen = strlen(buf);
	// if gets blank or invalid length 
	// line just skip it
	if (strslen < 2 || strslen >= MAXLINE)
	{
		return true;
	}

	memset(key, 0, sizeof(key));
	memset(value, 0, sizeof(value));

	sscanf(buf, "%s %s\n", key, value);

	if (strlen(value) < 0)
	{
		return true;
	}
	if (!strcmp(key, ConfigItems[0]))
	{
		strcpy(GS_acfg->ServAddr, value);
	}
	else if (!strcmp(key, ConfigItems[1]))
	{
		GS_acfg->ServPort = atoi(value);
	}
	else if (!strcmp(key, ConfigItems[2]))
	{
		strcpy(GS_acfg->UpdateServ, value);
	}
	else if (!strcmp(key, ConfigItems[3]))
	{
		GS_acfg->LocalPort = atoi(value);
	}

	return true;
}


void ShowConfig(AppConfig& GS_acfg)
{	
	printf("ServAddr: [%s]\n", GS_acfg.ServAddr);
	printf("ServPort: [%d]\n", GS_acfg.ServPort);
	printf("UpdtADDR: [%s]\n", GS_acfg.UpdateServ);
	printf("LocalPort: [%d]\n", GS_acfg.LocalPort);
}

#if 0
int main()
{
	AppConfig GS_acfg = {0};

	if (LoadConfig(CONFIG_PATH, MyParseFunc, &GS_acfg))
	{
		ShowConfig(GS_acfg);
	}

	return 0;
}
#endif