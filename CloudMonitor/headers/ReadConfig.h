#ifndef _ALBERT_READCONFIG__
#define _ALBERT_READCONFIG__

#define CONFIG_PATH "DATA\\config.ini"
#define MAXLINE     128

struct AppConfig
{
    char ServAddr[32];
    char UpdateServ[32];

    int  ServPort;

    // the struct is scalable
    // 支持额外的配置加入
};


// 回调函数,按行解析配置文件
typedef bool (*ParseFunCallback)(const char*, AppConfig *);

// 读取一个配置文件,解析其内容，保存在 AppConfig 结构体中
bool LoadKeywords(const char* ConfigFilePath, ParseFunCallback ParseFuncCallback, AppConfig *acfg);

void ShowConfig(AppConfig& acfg);

#endif
