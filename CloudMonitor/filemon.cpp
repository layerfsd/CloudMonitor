#include "network.h"
#include "FileMon.h"
#include "Encrypt.h"
#include "parsedoc.h"

#include <windows.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

using namespace std;


int DumpToFile(const char *FileName, char *buf, size_t FileSize)
{
	FILE     *fp;

	if ((fp = fopen(FileName, "wb")) == NULL)
	{
		return -1;
	}
	fwrite(buf, 1, FileSize, fp);

	fclose(fp);

	return 0;
}

int DumpFromFile(const char *FileName, char *buf, size_t FileSize)
{
	FILE     *fp;

	if ((fp = fopen(FileName, "rb")) == NULL)
	{
		return -1;
	}
	fread(buf, 1, FileSize, fp);

	fclose(fp);

	return 0;
}


int GetFileSize(const char *FileName, size_t *FileSize)
{
	FILE *fp;

	if ((fp = fopen(FileName, "rb")) == NULL)
	{
		//perror(FileName);  //����,��ʾ���������Ϣ
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	*FileSize = ftell(fp);

	fclose(fp);
	fp = NULL;

	return 0;
}


bool LoadHashList(string &path, vector<HashItem> hashList)
{
	FILE *fp = NULL;
	char *FileBuf = NULL;
	const char *FileName = path.c_str();

	if ((fp = fopen(FileName, "r")) == NULL)
	{
		perror(FileName);
		return false;
	}

	int			cnt = 0;
	HashItem	tp;

	while (!feof(fp))
	{
		cnt += 1;
		fscanf(fp, "%s\n", tp.path);
		fscanf(fp, "%s\n", tp.hash);
		fscanf(fp, "%s\n", tp.matchDetails);
		printf("%3d path:%s hash:%s details:%s\n", cnt, tp.path, tp.hash, tp.matchDetails);
		hashList.push_back(tp);
	}

	fclose(fp);
	return true;
}


bool StoreHashList(string &path, vector<HashItem> hashList)
{
	return true;
}


bool LoadKeywords(string  &filePath, vector<Keyword> &kw)
{
	FILE *fp = NULL;
	char *FileBuf = NULL;
	int   TotalLine = 0;
	char  buf[MAX_KEYWORD];
	const char *FileName = filePath.c_str();

	//GetFileSize(FileName, &FileSize);
	if ((fp = fopen(FileName, "r")) == NULL)
	{
		perror(FileName);
		return false;
	}
	
	//fscanf(fp, "%d\n", &TotalLine);
	//printf("Get TotalLine: %d\n", TotalLine);

	int   line = 0;
	Keyword tp;
	char	word[MAX_KEYWORD];

	//while (line < TotalLine)
	while (!feof(fp))
	{
		memset(buf, 0, MAX_KEYWORD);
		fgets(buf, MAX_KEYWORD, fp);
		if (strlen(buf) < 2)
		{
			break;
		}
		sscanf(buf, "%d-%s\n", &tp.rank, word);
		tp.word = word;
		//printf("rank: %d len: [%d] word: %s\n", tp.rank, tp.word.size(), word);
		kw.push_back(tp);
		line += 1;
	}

	fclose(fp);
	return true;
}



/*
return value:
n matched items
-1 error
0 none match
*/

void DumpByte(const char *str)
{
	while (*str)
	{
		printf("%02X ", *str & 0x000000FF);
		str++;
	}
	putchar('\n');
}


string GBKToUTF8(const char* strGBK)
{
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}

// ��ȡ�ؼ��ֵ�������
// ������� CONTEXT_LEN �ĳ���
void GetKeywordContext(char *FileBuf, int offset, int FileSize, Keyword& context)
{
	if (offset < 0 || FileSize < 0)
	{
		return;
	}
	if (offset >= FileSize || NULL == FileBuf)
	{
		return;
	}

	int prevPos = offset;
	// string.size() �� string.length() ������
	// size: ӵ�д洢�ռ��С
	// length: ��ǰ�ַ����ĳ���
	int tailPos = offset + context.word.length();
	int tp = 0;

	const int limitSize = (UTF8_CHINESE_LEN *  CONTEXT_LEN) >> 1;
	char ch;

	tp = prevPos - limitSize;
	int addition = 0;
	if (tp >= 0)
	{
		prevPos = tp;
	}
	else {
		prevPos = 0;
		addition = abs(tp);
	}

	// look tail
	tp = 0;
	tp = tailPos + limitSize;
	if (tp <= FileSize)
	{
		tailPos = tp;
	}
	else {
		tailPos = FileSize;
	}

	// ���Ѿ�ȷ���ļ�ĩβ�ˣ�Ϊ�λ�Ҫ���䶯����
	//tailPos += addition;



	for (int i = 0; i < UTF8_CHINESE_LEN; i++)
	{
		ch = FileBuf[i + prevPos];
		if ((ch & 0xe0) == 0xe0)
		{
			prevPos = i + prevPos;
			break;
		}
	}


	// ���α���ǰ�ƶ���һ��utf8����֮ǰ
	for (int i = 0; i < UTF8_CHINESE_LEN; i++)
	{
		ch = FileBuf[tailPos - i];
		if ((ch & 0xe0) == 0xe0)
		{
			tailPos = tailPos - i -3;
			break;
		}
	}

	// �����ַ����ϵ�
	tp = FileBuf[tailPos];
	FileBuf[tailPos] = 0;
	//printf(FileBuf + prevPos);

	// �ѻس�ת��Ϊ�ո�
	// ת��'\n' '|'�ַ���ԭ���ǣ�ͨ��Э��ʹ������Ϊ�ָ���
	for (int i = 0; i < tailPos; i++)
	{
		if ('\r' == FileBuf[i] || '\n' == FileBuf[i])
		{
			FileBuf[i] = '_';
		}
		else if ('|' == FileBuf[i])
		{
			FileBuf[i] = 'I';
		}
	}


	context.context = FileBuf + prevPos;

	// ��ԭ
	FileBuf[tailPos] = tp;
	//cout << context.context << endl;
}



int KeywordFilter(vector<Keyword> &kw, char *FileName, string &message, char *context=NULL)
{
	char    *FileBuf;
	size_t  FileSize = 0;
	size_t  offset = 0;
	int     nmatch = 0;

	if (GetFileSize(FileName, &FileSize) != 0)
	{
		// printf("GetFileSize error");
		return -1;
	}
	if ((FileBuf = (char *)malloc(FileSize)) == NULL)
	{
		perror("FileBuf");
		return -1;
	}

	if (DumpFromFile(FileName, FileBuf, FileSize) != 0)
	{
		perror("DumpFileByte error");
		return -1;
	}

	offset = 0;
	int WordLen = 0;
	map<Keyword, int> MatchRecord;

	while (offset < FileSize)
	{
		for (unsigned int i = 0; i != kw.size(); i += 1)
		{
			/*
			Ϊ������ܣ��˴������Լ�ʵ��һ�� memcmp���Լ��ٶ�ջ���ÿ���
			ͬʱ�ı��������ƥ�䷽ʽΪ���Ӹ�Ч���ַ���ƥ���㷨
			*/
			//printf("keyword: [%s] wordsize[%d]\n", kw[i].word.c_str(), kw[i].word.size());
			if (!memcmp(kw[i].word.c_str(), FileBuf + offset, kw[i].word.size()))
			{
				MatchRecord[kw[i]]++;
				nmatch += 1;
				//printf("%s matched in offset %u!!!\n", kw[i].word.c_str(), offset);

				// ��ȡ��һ�����ظ��ؼ��ֵ�������
				if (1 == MatchRecord[kw[i]]) {
					GetKeywordContext(FileBuf, offset, FileSize, kw[i]);
				}
			}
		}
		offset += 1;
	}

	// ���ɱ�����־
	char tmp[MAX_LOG_SIZE];

	map<Keyword, int>::const_iterator ite;
	message.clear();

	// ����һ������Ĳ����ڵĹؼ��ֵȼ�
	// �ҳ�rank��С�Ĺؼ���
	int whichWord = 10;
	for (ite = MatchRecord.begin(); ite != MatchRecord.end(); ite++)
	{
		if (ite->first.rank < whichWord) {
			whichWord = ite->first.rank;
		}
		memset(tmp, 0, MAX_LOG_SIZE);
		sprintf(tmp, "%d-%s-%d ", (int)(ite->first.rank), ite->first.word.c_str(), ite->second);
		message += tmp;
	}

	// ������־����: ��־���� + ��־����(�ؼ���������)
	if (NULL == context)
	{
		message += "\n";
		message += to_string(OPEN_FILE_WHILE_ONLINE);
		message += " ";
	}

	//message += "ThisIsContext";

	// �ҳ���Сrank��Ӧ��������
	for (auto& i : kw)
	{
		if (i.context.size() && i.rank == whichWord) {
			if (NULL == context)
			{
				message += i.context;
			}
			else
			{
				strncpy(context, i.context.c_str(), MAX_CONTEXT);
			}
			printf("[MatchContext] %s size: %d word: %s rank: %d\n", \
				i.context.c_str(), i.context.size(), i.word.c_str(), i.rank);
			break;
		}
	}

	for (auto& i : kw)
	{
		i.context.clear();
	}

	free(FileBuf);
	return nmatch;
}

bool HashFile(const char *fileName, char *buf)
{
	MD5_CTX c;
	unsigned char md5[17] = { 0 };
	const char *set = "0123456789abcdef";
	int cnt = 0;

	size_t fileSize = 0;

	if (0 != GetFileSize(fileName, &fileSize))
	{
		// file not exists
		cout << "file not exists " << fileName << endl;
		return false;
	}

	char *pData = NULL;

	if ((pData = (char*)malloc(fileSize)) == NULL) {
		// alloc memory error
		return false;
	}

	MD5_Init(&c);

	if (0 != DumpFromFile(fileName, pData, fileSize))
	{
		return false;
	}
	MD5_Update(&c, pData, fileSize);

	MD5_Final(md5, &c);
	free(pData);

	/* convert md5hash to string */
	cnt = 0;
	while (cnt < MD5_DIGEST_LENGTH)
	{
		buf[cnt * 2] = set[md5[cnt] >> 4];
		buf[cnt * 2 + 1] = set[md5[cnt] & 0xF];
		cnt += 1;
	}
	/* end convert */

	return true;
}


//����һ���ļ�
bool wrapEncreytFile(SFile& sf)
{
	
	if (!EncreptFile(sf.savedPath.c_str(), sf.encPath.c_str(), sf.encPasswd.c_str()))
	{
		return false;
	}
	// ��ȡ���ܺ���ļ���С
	if (0 != GetFileSize(sf.encPath.c_str(), &(sf.encSize)))
	{
		cout << "GetFileSize() error!" << endl;
		return false;
	}

	return true;
}

bool GetNameFromPath(const char* src, char* dst)
{
	if (NULL == src || NULL == dst)
	{
		return false;
	}
	const char *pre = NULL;
	const char *tail = NULL;

	tail = strrchr(src, '.');
	if (NULL == tail)
	{
		return false; //�Ҳ����ļ���׺�����޷��ж��ļ�����
	}


	pre = strrchr(src, '\\');
	if (NULL == pre)
	{
		pre = src;
	}
	else
	{
		pre++;
	}

	while (pre != tail)
	{
		*dst++ = *pre++;
	}
	*dst = 0;

	return true;
}


bool initSFile(SFile &sf)
{

	// ��ȫ·���л�ȡ�ļ���
	// eg: D:\work\test.docx --> test.docx
	sf.fileName = sf.localPath.substr(sf.localPath.rfind('\\') + 1);

	// ������ʱ����·��
	// TMP\test.docx
	// all2txt ����ò���޷�����ʶ��·��
	sf.savedPath = TMP_DIR + sf.fileName;

	char tmpName[1024];
	memset(tmpName, 0, sizeof(tmpName));
	if (!GetNameFromPath(sf.fileName.c_str(), tmpName))
	{
		return false;
	}
	else
	{
		// ������ʱ�ļ��� eg: tmp\test.txt
		sf.txtName = tmpName;
		sf.txtName += TXT_SUFFIX;
	}



	// ��Ŀ���ļ���������ʱĿ¼
	if (!CopyFile(sf.localPath.c_str(), sf.savedPath.c_str(), FALSE))
	{
		//perror("CopyFile Error: \n");
		return false;
	}


	char buf[HASH_SIZE+1];

	if (0 != GetFileSize(sf.savedPath.c_str(), &sf.fileSize))
	{
		cout << "GetFileSize() error!" << endl;
		return false;
	}

	memset(buf, 0, HASH_SIZE + 1);
	if (HashFile(sf.savedPath.c_str(), buf))
	{
		sf.fileHash = buf;
		//cout << "HashFile() " << sf.localPath << sf.fileHash << endl;
	}
	else
	{
		//cout << "HashFile() " << sf.localPath << " error" << endl;
		return false;
	}

	// ������ʱ�ļ���·�� eg: D:\TMP\test.docx.txt
	//sf.txtPath = TMP_DIR + sf.txtName;
	sf.txtPath = TMP_DIR + sf.txtName;

	// ���ɼ����ļ���·�� eg: TMP\test.docx.aes
	sf.encSrc = GBKToUTF8(sf.savedPath.c_str());
	sf.encName = sf.fileName + ".aes";
	sf.encPath = TMP_DIR + sf.encName;

	// ���ü����ļ�������Ϊ�����ļ��Ĺ�ϣ����(��ϣ����Ϊ32Bytes)
	sf.encPasswd = sf.fileHash.substr(6, 14);


	// ���ڷ���˲��� utf-8 ���룬���ת���ļ�������,  gbk -> utf8 
	// ��ֹ�������ʾ�ϴ����ļ���Ϊ����
	sf.fileName = GBKToUTF8(sf.fileName.c_str());
	sf.utf8Path = GBKToUTF8(sf.localPath.c_str());
	sf.encName = GBKToUTF8(sf.encName.c_str());
	return true;
}


inline void _showSFile(SFile &sf)
{
	cout << "savedPath: " << sf.savedPath << endl;
	cout << "fileName: " << sf.fileName << endl;
	cout << "fileSize: " << sf.fileSize << endl;

	cout << "hash: " << sf.fileHash << endl;

	cout << "txtName: " << sf.txtName << endl;
	cout << "txtPath: " << sf.txtPath << endl;

	cout << "encName: " << sf.encName << endl;
	cout << "encPath: " << sf.encPath << endl;
	cout << "encSize: " << sf.encSize << endl;
}


bool fsFilter(SFile &sf, vector<Keyword> &kw, vector<HashItem> &hashList, \
	string &message, char *context)
{	
	if (-1 ==_access(sf.localPath.c_str(), 0))
	{
		cout << sf.localPath << " not exists!!!" << endl;
		return false;
	}
	if (kw.size() <= 0)
	{
		cout << "Empty KeywordList..." << endl;
		// �ֵ�Ϊ��
		return false;
	}

	if (!initSFile(sf))
	{
		cout << "initSfile Failed!" << endl;
		return false;
	}
	char localPath[_MAX_PATH], txtPath[_MAX_PATH];
	
	strncpy(localPath, sf.savedPath.c_str(), _MAX_PATH);
	strncpy(txtPath, sf.txtPath.c_str(), _MAX_PATH);

	// ����ļ�����ת��ʧ�ܣ�������������ļ�
	if (0 != ParseFile2Text(localPath, txtPath))
	{
		cout << "[ParseFile2Text] FAILED" << sf.localPath << endl;
		return false;
	}

	// ���û��ƥ�䵽�ؼ��֣�����Ը��ļ�
	if (KeywordFilter(kw, txtPath, message, context) <= 0)
	{
		cout << "Find nothing from: " << sf.localPath << endl;
		return false;
	}
	else
	{
		return true;
	}
}


int GB2312ToUtf8(const char* gb2312, char* utf8)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);

	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);

	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);

	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, len, NULL, NULL);

	if (wstr) delete[] wstr;

	return len;
}

int DecodeGB2312ToUtf8(const char* FileName)
{
	size_t FileSize = 0;
	int    ret;
	char *buf1 = NULL;
	char *buf2 = NULL;

	GetFileSize(FileName, &FileSize);

	// �����ܿռ� = ucs2_FileSize + 1.5 * ucs_FileSize = 2.5 *ucs_FileSize
	// Ϊ��֤³����,���������ռ�
	if ((buf1 = (char *)malloc(FileSize * 3)) == NULL)
	{
		perror("malloc");
		exit(1);
	}
	buf2 = buf1 + FileSize + 1;

	DumpFromFile(FileName, buf1, FileSize);
	buf1[FileSize] = 0;

	size_t fs2 = GB2312ToUtf8(buf1, buf2);
	ret = DumpToFile(FileName, buf2, fs2 - 1);

	free(buf1);
	return 0;
}


typedef unsigned char Uchar;
bool IsUtf8(const char* FileName)
{
	FILE *fp = NULL;
	size_t FileSize = 0;
	char *fileBuf = NULL;


	GetFileSize(FileName, &FileSize);
	fileBuf = (char *)malloc(FileSize);
	DumpFromFile(FileName, fileBuf, FileSize);

	size_t i = 0;
	bool ret = true;

	for (; ret && (i < FileSize); i++)
	{
		Uchar hexchar = fileBuf[i];
		// ignore ascii code
		if (!(hexchar & 0x80))
		{
			continue;
		}

		// calculate how many serial "1"
		int   BitOneCount = 0;
		Uchar num = hexchar;
		while (num & 0x80)
		{
			if (num & 0x80)
			{
				BitOneCount += 1;
			}
			num <<= 1;
		}

		BitOneCount -= 1;
		while (BitOneCount > 0)
		{
			i += 1;
			num = fileBuf[i];   // num suppose to be 10xx xxxx
			num >>= 6;		    // num = 0000 0010
			if (2 != num)
			{
				ret = false;
				//printf("i = %d num = %d hexchar = 0x%x BitOneCount= %d\n", i, num, hexchar, BitOneCount);
				break;
			}
			BitOneCount -= 1;
		}

		//end for
	}


	free(fileBuf);
	return ret;
}

bool isContinue(const char* lPath, int length)
{
	return true;
}