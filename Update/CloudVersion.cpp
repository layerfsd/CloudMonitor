#include "stdafx.h"
#include "CloudVersion.h"
#include "manage.h"

CloudVersion::CloudVersion()
{
	memset(this->workPath, 0, sizeof(this->workPath));
	SetWorkPath(this->workPath);

	// ���ر���hash�б�
	LoadHashList(LOCAL_HASHLIST, this->localHashList);
	if (-1 == _access(TMPDOWN_DIR, 0))
	{
		_mkdir(TMPDOWN_DIR);
	}
}

double CloudVersion::GetCurVersion()
{
	FILE *fp;

	if ((fp = fopen(VERSION_FILE, "r")) == NULL)
	{
		//perror(FileName);  //����,��ʾ���������Ϣ
		return INVALID_VERSION;
	}

	fscanf(fp, "%lf", &this->CurVersion);

	fclose(fp);
	return this->CurVersion;
}

bool CloudVersion::SetLatestVersion2File()
{
	FILE *fp;

	if ((fp = fopen(VERSION_FILE, "w")) == NULL)
	{
		//perror(FileName);  //����,��ʾ���������Ϣ
		return false;
	}

	fprintf(fp, "%s", this->LatestVersionStr.c_str());

	fclose(fp);
	return true;
}

bool CloudVersion::GetLatestVersion()
{
	// �ӷ���˻�ȡ���汾�嵥���ļ�
	if (!GetFilesList(UPDATE_URL, TMPFILE_NAME))
	{
		return false;
	}

	FILE *fp;

	if ((fp = fopen(TMPFILE_NAME, "r")) == NULL)
	{
		//perror(FileName);  //����,��ʾ���������Ϣ
		return false;
	}

	char	verBuf[16];		// x.y min lenght = 3, max
	double	tmpVer = 0;
	int		bufLen = 0;

	while (!feof(fp))
	{
		memset(verBuf, 0, sizeof(verBuf));

		fgets(verBuf, sizeof(verBuf)-1, fp);
		bufLen = strnlen(verBuf, sizeof(verBuf));

		//printf("getline[%d]: %s\n", bufLen, verBuf);

		if (bufLen < 3 || bufLen > 15)
		{
			continue;
		}
		else
		{
			sscanf(verBuf + 1, "%lf", &tmpVer);
			//printf("parsing %s ---> %lf\n", verBuf, tmpVer);
		}
		if (tmpVer > this->LatestVersion)
		{
			this->LatestVersion = tmpVer;
			this->LatestVersionStr = verBuf;
			// ɾ����β'\n'
			size_t n = this->LatestVersionStr.find_first_of('\n');
			if (n != string::npos)
			{
				this->LatestVersionStr.erase(n);
			}
		}
	}

	fclose(fp);

	return true;
}

bool CloudVersion::WhetherUpdate()
{
	return this->LatestVersion > this->CurVersion;
}

bool CloudVersion::RequestHashList()
{
	string url = UPDATE_URL;

	// ���ݡ����°汾�š�ƴ�� url
	url += this->LatestVersionStr;
	url += "/";
	url += "hashlist.txt";

	FtpFile ftpfile = {
		TMP_HASHLIST, // name to store the file as if succesful//  
		NULL
	};

	if (0 != DownloadFtpFile(url.c_str(), ftpfile))
	{
		return false;
	}

	return LoadHashList(ftpfile.filename, this->remotHashList);
}

bool CloudVersion::DownloadLatestFiles(const char* keepDir)
{
	if (this->remotHashList.size() <= 0)
	{
		return false;
	}

	// �ҵ���һ�µ��ļ�
	for (auto i = this->remotHashList.begin(); i != this->remotHashList.end(); i++)
	{
		// ��������ļ���Ӧ��MD5��Զ���ļ���Ӧ��MD5��һ�£��򽫸��ļ�·�����������б�
		if (this->localHashList[i->first] != this->remotHashList[i->first])
		{
			this->downloadSet.insert(i->second);
		}
	}

	string baseUrl = UPDATE_URL;
	baseUrl += this->LatestVersionStr;
	baseUrl += "/";

	string basePath = keepDir;
	basePath += "/";

	string curUrl, curPath, tpName, tpDir;

	for (auto i = this->downloadSet.begin(); i != this->downloadSet.end(); i++)
	{
		tpName = (*i).substr(2);
		curUrl = baseUrl + tpName;
		curPath = basePath + tpName;
		FtpFile tfile{ curPath.c_str(), NULL };

		//printf("[%s] --> [%s]\n", curUrl.c_str(), curPath.c_str());

		size_t pre = curPath.find_first_of('/');
		size_t tail = curPath.find_last_of('/');
		
		if (pre != tail)
		{
			tpDir = curPath.substr(0, tail);
			if (0 != _access(tpDir.c_str(), 0))
			{
				printf("_makedir [%s]\n", tpDir.c_str());
				_mkdir(tpDir.c_str());
			}
		}

		DownloadFtpFile(curUrl.c_str(), tfile);
	}

	return false;
}

bool CloudVersion::ReplaceFiles(const char * keepDir)
{

	return false;
}

CloudVersion::~CloudVersion()
{
	// ɾ����ʱ�ļ�
	system("DEL /f " TMPFILE_NAME);
	system("DEL /f " TMP_HASHLIST);
	system("RD /q /s " TMPDOWN_DIR);
}
