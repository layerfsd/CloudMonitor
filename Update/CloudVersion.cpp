#include "stdafx.h"
#include "CloudVersion.h"
#include "manage.h"

#define MAX_RETRY_TIME	3

const char* result[]{
	"[FAILED]", "[OK]"
};


CloudVersion::CloudVersion()
{
	memset(this->workPath, 0, sizeof(this->workPath));
	printf("SetWorkPath\n");
	SetWorkPath(this->workPath);

	// ���ر���hash�б�
	printf("Loading Local HashList\n");
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

	// skip 'v'
	fseek(fp, 1, SEEK_CUR);
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

	printf("Set Latest Versio [%s] to [%s] Done\n", this->LatestVersionStr.c_str(), VERSION_FILE);
	return true;
}

bool CloudVersion::GetLatestVersion()
{
	printf("Getting Lastest Version\n");
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
			// skip 'v'
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
	cout << "Latest Version: " << this->LatestVersionStr << endl;
	fclose(fp);

	return true;
}

bool CloudVersion::WhetherUpdate()
{
	printf("LatestVersion: [%lf] CurrentVersion [%lf]\n", this->LatestVersion, this->CurVersion);
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
	printf("RequestHashList: \n");
	printf("[%s] [%s]\n", url.c_str(), ftpfile.filename);
	if (!DownloadFtpFile(url.c_str(), ftpfile))
	{
		return false;
	}

	return LoadHashList(ftpfile.filename, this->remotHashList);
}

inline void CheckPathExists(string& curPath)
 {
	size_t pre = curPath.find_first_of('/');
	size_t tail = curPath.find_last_of('/');
	string tpDir;

	if (pre != tail)
	{
		tpDir = curPath.substr(0, tail);
		if (0 != _access(tpDir.c_str(), 0))
		{
			printf("_makedir [%s]\n", tpDir.c_str());
			_mkdir(tpDir.c_str());
		}
	}
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
			this->downloadSet.insert(i->first);
		}
	}

	string baseUrl = UPDATE_URL;
	baseUrl += this->LatestVersionStr;
	baseUrl += "/";

	string basePath = keepDir;
	basePath += "/";

	string curUrl, curPath, tpName;
	bool  downloadStatus = false;
	DWORD dwMaxRetryTime = 0;

	for (auto i = this->downloadSet.begin(); i != this->downloadSet.end(); i++)
	{
		tpName = (*i);
		curUrl = baseUrl + tpName;
		curPath = basePath + tpName;
		FtpFile tfile{ curPath.c_str(), NULL };

		//printf("[%s] --> [%s]\n", curUrl.c_str(), curPath.c_str());
		CheckPathExists(curPath);
		printf("[%s] [%s]\n", curUrl.c_str(), curPath.c_str());
		
		downloadStatus = DownloadFtpFile(curUrl.c_str(), tfile);
		dwMaxRetryTime = 0;

		// ��������ļ�ʧ�ܣ��������Գ�������
		while ((dwMaxRetryTime < MAX_RETRY_TIME) && (downloadStatus != TRUE))
		{
			dwMaxRetryTime += 1;
			printf("[%d] download [%s] failed\n", dwMaxRetryTime, tfile.filename);
			downloadStatus = DownloadFtpFile(curUrl.c_str(), tfile);
		}
	}

	return downloadStatus;
}

bool CloudVersion::ReplaceFiles(const char * keepDir)
{	
	// TODO
	// У�������ļ���������

	string baseDir = TMPDOWN_DIR;
	baseDir += "/";
	string tpName, srcName;
	BOOL   bRet = FALSE;
	for (auto i = this->downloadSet.begin(); i != this->downloadSet.end(); i++)
	{
		tpName = (*i);
		srcName = baseDir + tpName;
		printf("MoveFile [%s] to [%s]\n", srcName.c_str(), tpName.c_str());
		bRet = MoveFileEx(srcName.c_str(), tpName.c_str(), MOVEFILE_REPLACE_EXISTING);
		printf("%s\n", result[bRet]);
	}

	// �����°汾��д�뵽�����ļ�
	if (!this->SetLatestVersion2File())
	{
		printf("Set Latest Version To File failed.\n");
		bRet = FALSE;
	}

	return B2b(bRet);
}
CloudVersion::~CloudVersion()
{
	// ɾ����ʱ�ļ�
	printf("\n\nCleanning Temporary Files...\n");
	system(DELETE_FILE_CMD TMPFILE_NAME);
	system(DELETE_FILE_CMD TMP_HASHLIST);
	system(DELETE_DIRS_CMD TMPDOWN_DIR);
}

void IsFileExists(map<string,string>& fileList)
{
	if (fileList.size() <= 0)
	{
		return;
	}

	for (auto i = fileList.begin(); i != fileList.end(); i++)
	{
		if (-1 == _access(TMPDOWN_DIR, 0))
		{
			_mkdir(TMPDOWN_DIR);
		}
	}
}