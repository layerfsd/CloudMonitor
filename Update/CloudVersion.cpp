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

	// 加载本地hash列表
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
		//perror(FileName);  //调试,显示具体出错信息
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
		//perror(FileName);  //调试,显示具体出错信息
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
	// 从服务端获取‘版本清单’文件
	if (!GetFilesList(UPDATE_URL, TMPFILE_NAME))
	{
		return false;
	}

	FILE *fp;

	if ((fp = fopen(TMPFILE_NAME, "r")) == NULL)
	{
		//perror(FileName);  //调试,显示具体出错信息
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
			// 删除行尾'\n'
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

	// 根据‘最新版本号’拼接 url
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

	// 找到不一致的文件
	for (auto i = this->remotHashList.begin(); i != this->remotHashList.end(); i++)
	{
		// 如果本地文件对应的MD5与远程文件对应的MD5不一致，则将该文件路径加入下载列表
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

		// 如果下载文件失败，则最多可以尝试三次
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
	// 校验下载文件的完整性

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

	// 把最新版本号写入到本地文件
	if (!this->SetLatestVersion2File())
	{
		printf("Set Latest Version To File failed.\n");
		bRet = FALSE;
	}

	return B2b(bRet);
}
CloudVersion::~CloudVersion()
{
	vector<string> pathList{
		TMPFILE_NAME,
		TMP_HASHLIST,
		TMPDOWN_DIR,
	};

	// 删除临时文件
	printf("\n\nCleanning Temporary Files...\n");
	DeleteFiles(pathList);
}
