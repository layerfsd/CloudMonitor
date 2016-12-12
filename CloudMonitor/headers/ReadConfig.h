#ifndef _ALBERT_READCONFIG__
#define _ALBERT_READCONFIG__

#define CONFIG_PATH "DATA\\config.ini"
#define MAXLINE     128

struct AppConfig
{
	char ServAddr[32];
	char UpdateServ[32];

	int  ServPort;
	
};


typedef bool(*ParseFunCallback)(const char*, AppConfig *);

bool LoadConfig(const char* ConfigFilePath, ParseFunCallback ParseFuncCallback, AppConfig *acfg);

void ShowConfig(AppConfig& acfg);

bool MyParseFunc(const char* buf, AppConfig* acfg);

#endif
