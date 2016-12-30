#ifndef _ALBERT_READCONFIG__
#define _ALBERT_READCONFIG__

#define CONFIG_PATH "DATA\\config.ini"
#define MAXLINE     128

// ����ṹ���������桰DATA\config.ini���е�������Ϣ
struct AppConfig
{
	char ServAddr[32];	  // Զ�̷����IPv4��ַ
	char UpdateServ[32];  // '���·�����'IPv4��ַ

	int  ServPort;		  // Զ�̷���˼����˿�
	int  LocalPort;		  // ����TCPͨ�Ŷ˿�
};


typedef bool(*ParseFunCallback)(const char*, AppConfig *);

bool LoadConfig(const char* ConfigFilePath, ParseFunCallback ParseFuncCallback, AppConfig *GS_acfg);

void ShowConfig(AppConfig& GS_acfg);

bool MyParseFunc(const char* buf, AppConfig* GS_acfg);

#endif
