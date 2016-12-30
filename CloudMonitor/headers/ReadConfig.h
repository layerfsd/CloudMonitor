#ifndef _ALBERT_READCONFIG__
#define _ALBERT_READCONFIG__

#define CONFIG_PATH "DATA\\config.ini"
#define MAXLINE     128

// 这个结构体用来保存“DATA\config.ini”中的配置信息
struct AppConfig
{
	char ServAddr[32];	  // 远程服务端IPv4地址
	char UpdateServ[32];  // '更新服务器'IPv4地址

	int  ServPort;		  // 远程服务端监听端口
	int  LocalPort;		  // 本地TCP通信端口
};


typedef bool(*ParseFunCallback)(const char*, AppConfig *);

bool LoadConfig(const char* ConfigFilePath, ParseFunCallback ParseFuncCallback, AppConfig *GS_acfg);

void ShowConfig(AppConfig& GS_acfg);

bool MyParseFunc(const char* buf, AppConfig* GS_acfg);

#endif
