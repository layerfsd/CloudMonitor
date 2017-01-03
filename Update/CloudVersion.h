#pragma once

#define INVALID_VERSION 0

struct HashItem
{
	char fileName[MAX_PATH];
	char md5[33];
};

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

private:
	double CurVersion;
	double LatestVersion;
	string LatestVersionStr;

	vector<HashItem> localHashList;
	vector<HashItem> remotHashList;
};

