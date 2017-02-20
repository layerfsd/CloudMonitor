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

	// ������Ҫ�滻���ļ�
	void BackUpOldFiles();

	// ��ԭ���ݵ��ļ�
	void RollBack();

private:
	double CurVersion;
	double LatestVersion;
	string LatestVersionStr;

	time_t startTime;	// ��¼��������ʱ��
	time_t endTime;		// ��¼�������н���ʱ��

	char   workPath[MAX_PATH];

	map<string, string> localHashList;		// Զ���嵥�б�
	map<string, string> remotHashList;		// �����嵥�б�
	map<string, string> downloadList;		// ��Ҫ���ص��ļ�

	vector<string> replaceList;		// ��Ҫ�滻���ļ�

};

