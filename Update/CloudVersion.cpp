#include "stdafx.h"
#include "CloudVersion.h"
#include "manage.h"

#define MAX_RETRY_TIME	3

const char* result[]{
	"[FAILED]", "[OK]"
};

extern Config GlobalConfig;


CloudVersion::CloudVersion()
{
	memset(this->workPath, 0, sizeof(this->workPath));
	printf("SetWorkPath\n");
	SetWorkPath(this->workPath);

	// ������־�ļ�����ʾ�������Ը��£���������
	fopen(UPDATE_CHECKED_FLAG, "w");
	WriteToLog("[UPDATE] " "CREATE " UPDATE_CHECKED_FLAG);

	// ���ر���hash�б�
	printf("Loading Local HashList\n");
	LoadHashList(LOCAL_HASHLIST, this->localHashList);

	if (!LoadConfig())
	{
		cout << "LoadConfig Failed" << endl;
		exit(1);
	}

	if (-1 == _access(TMPDOWN_DIR, 0))
	{
		_mkdir(TMPDOWN_DIR);
	}

	WriteToLog("[Update.exe] Started By Service");
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
	if (!GetFilesList(GlobalConfig.update_url, TMPFILE_NAME))
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
	//printf("LatestVersion: [%lf] CurrentVersion [%lf]\n", this->LatestVersion, this->CurVersion);
	return this->LatestVersion > this->CurVersion;
}

bool CloudVersion::RequestHashList()
{
	string url = GlobalConfig.update_url;

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


bool CloudVersion::DownloadLatestFiles(const char* keepDir)
{
	// �ҵ���һ�µ��ļ�
	for (auto i : remotHashList)
	{
		// ��������ļ���Ӧ��MD5��Զ���ļ���Ӧ��MD5��һ�£��򽫸��ļ�·�����������б�

		if (this->localHashList[i.first] != i.second)
		{
			this->downloadList[i.first] = i.second;
			cout << "ori: [" << this->localHashList[i.first] << "] new: [" << i.second << "] name: " << i.first << endl;
		}
	}

	string baseUrl = GlobalConfig.update_url;
	baseUrl += this->LatestVersionStr;
	baseUrl += "/";

	string basePath = keepDir;
	basePath += "/";

	string curUrl, curPath, tpName, curMd5;
	bool  downloadStatus = true;
	DWORD dwMaxRetryTime = 0;

	for (auto i : this->downloadList )
	{
		tpName = i.first;
		curMd5 = i.second;
		curUrl = baseUrl + tpName;
		curPath = basePath + tpName;
		FtpFile tfile{ curPath.c_str(), NULL };

		printf("[%s] --> [%s]\n", curUrl.c_str(), curPath.c_str());

		CheckPathExists(curPath);
		
		downloadStatus = DownloadFtpFile(curUrl.c_str(), tfile);
		dwMaxRetryTime = 0;

		// ��������ļ�ʧ�ܣ��������Գ�������
		while ((dwMaxRetryTime < MAX_RETRY_TIME) && (downloadStatus != TRUE))
		{
			dwMaxRetryTime += 1;
			printf("[%d] download [%s] failed\n", dwMaxRetryTime, tfile.filename);
			downloadStatus = DownloadFtpFile(curUrl.c_str(), tfile);
		}

		// �ļ����ء��ɹ���������У����ļ�
		// ������ص��ļ�MD5�Ƿ������ڡ�Զ���嵥�ļ����ϵ�MD5һ�£������һ��˵������ļ�����ȷ
		if (downloadStatus && !IsFileHashEqual(curPath, curMd5))
		{
			downloadStatus = false;
			break;
		}
	}

	return downloadStatus;
}

bool CloudVersion::ReplaceFiles(const char * keepDir)
{	
	this->BackUpOldFiles();	// ���滻�ļ�֮ǰ���ȱ��ݽ�Ҫ�滻���ļ�

	string baseDir = TMPDOWN_DIR;
	baseDir += "/";
	string tpName, srcName;
	BOOL   bRet = FALSE;

	for (auto i : this->downloadList)
	{
		tpName = i.first;
		srcName = baseDir + tpName;
		printf("MoveFile [%s] to [%s] ", srcName.c_str(), tpName.c_str());
		bRet = MoveFileExA(srcName.c_str(), tpName.c_str(), MOVEFILE_REPLACE_EXISTING);
		printf("%s\n", result[bRet]);

		// ��������滻�������˳�
		if (!bRet)
		{
			break;
		}
	}


	// �����°汾��д�뵽�����ļ�
	if (bRet && !this->SetLatestVersion2File())
	{
		printf("Set Latest Version To File failed.\n");
		bRet = FALSE;
	}

	if (bRet)
	{
		bRet = MoveFileExA(TMP_HASHLIST, LOCAL_HASHLIST, MOVEFILE_REPLACE_EXISTING);
	}

	return B2b(bRet);
}


void CloudVersion::BackUpOldFiles()
{
	const char* path;
	string baseDir = TMP_BACKUPDIR;
	baseDir += "/";

	string bakPath;
	
	cout << "\n\nBackupFiles ..." << endl;
	for (auto i : this->downloadList)
	{
		path = i.first.c_str();
		bakPath = baseDir + path;

		if (_access(path, 0) == 0)
		{
			this->replaceList.push_back(path);
			
			CheckPathExists(path);
			cout << "Copy [" << path << "] [" << bakPath << "] " << endl;
			CopyFileA(path, bakPath.c_str(), FALSE);
		}
	}
}


void CloudVersion::RollBack()
{
	const char* path;
	string baseDir = TMP_BACKUPDIR;
	string bakPath;

	cout << "\n\nRollbacking ..." << endl;
	for (auto i : this->replaceList)
	{
		path = i.c_str();
		bakPath = baseDir + path;

		if (_access(bakPath.c_str(), 0) == 0)
		{
			CheckPathExists(path);  // ȷ�����Ը���
			cout << "Copy [" << bakPath << "] [" << path << "] " << endl;
			CopyFileA(bakPath.c_str(), path, FALSE);	// FALSE����: Ϊ���ǿ���
		}
	}

}

CloudVersion::~CloudVersion()
{
	vector<string> pathList{
		TMPFILE_NAME,
		TMP_HASHLIST,
		TMPDOWN_DIR,
		TMP_BACKUPDIR,
	};

	// ɾ����ʱ�ļ�
	printf("\n\nCleanning Temporary Files...\n");
	DeleteFiles(pathList);


	WriteToLog("[Update.exe] Created " UPDATE_CHECKED_FLAG);
	WriteToLog("[Update.exe] try start Service");
	WriteToLog("[Update.exe] exit.");

	// �����ں�̨����ʱ���Ż᳢�Կ�������
	if (GlobalConfig.enableLog)
	{
		system("sc start CloudMonitorService");
	}
}
