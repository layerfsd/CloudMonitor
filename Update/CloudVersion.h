#pragma once

#include "manage.h"

#define INVALID_VERSION 0


class CloudVersion
{
public:
	CloudVersion();
	~CloudVersion();

	// ��ȡ������ĵ�ǰ�汾
	double GetCurVersion();

	// ���¹��̽�����ˢ�±��ء��汾��¼�ļ���
	bool SetLatestVersion2File();

	// �ӷ�������ȡ���°汾���
	bool GetLatestVersion();
	
	// �ȽϷ�������°汾�뵱ǰ�汾���룬�����Ƿ�Ҫ��������
	bool WhetherUpdate();

	// �����ȡ���������°汾�ġ���ϣ�б��ļ�
	bool RequestHashList();


	// �����ļ���ϣ��ͬ����ȡ���С����¡����ļ�
	bool DownloadLatestFiles(const char* keepDir);


	bool ReplaceFiles(const char* keepDir);

private:
	double CurVersion;
	double LatestVersion;
	string LatestVersionStr;
	char   workPath[MAX_PATH];

	map<string, string> localHashList;
	map<string, string> remotHashList;
	set<string>      downloadSet;
};

