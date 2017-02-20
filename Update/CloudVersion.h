#pragma once

#include "manage.h"

#define INVALID_VERSION 0

class CloudVersion
{
public:
	CloudVersion();
	~CloudVersion();

	// 获取本程序的当前版本
	double GetCurVersion();

	// 更新过程结束后，刷新本地‘版本记录文件‘
	bool SetLatestVersion2File();

	// 从服务器获取最新版本编号
	bool GetLatestVersion();
	
	// 比较服务端最新版本与当前版本号码，决定是否要继续升级
	bool WhetherUpdate();

	// 请求获取服务器最新版本的‘哈希列表’文件
	bool RequestHashList();


	// 根据文件哈希不同，获取所有‘更新’的文件
	bool DownloadLatestFiles(const char* keepDir);


	bool ReplaceFiles(const char* keepDir);

	// 备份需要替换的文件
	void BackUpOldFiles();

	// 还原备份的文件
	void RollBack();

private:
	double CurVersion;
	double LatestVersion;
	string LatestVersionStr;

	time_t startTime;	// 记录程序启动时间
	time_t endTime;		// 记录程序运行结束时间

	char   workPath[MAX_PATH];

	map<string, string> localHashList;		// 远程清单列表
	map<string, string> remotHashList;		// 本地清单列表
	map<string, string> downloadList;		// 将要下载的文件

	vector<string> replaceList;		// 将要替换的文件

};

