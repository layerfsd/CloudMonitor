#include <stdio.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <Assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <set>

#define WIRELESS_TYPE	71
#define WIRED_TYPE		6
using namespace std;

#pragma comment(lib, "iphlpapi.lib")

bool getMAC(vector<string>& mList);

bool getMAC(vector<string>& mList)
{
	PIP_ADAPTER_INFO AdapterInfo;
	DWORD dwBufLen = sizeof(AdapterInfo);
	char mac_addr[17];

	AdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
	if (AdapterInfo == NULL) {
		printf("Error allocating memory needed to call GetAdaptersinfo\n");

	}

	// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen     variable
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {

		AdapterInfo = (IP_ADAPTER_INFO *)malloc(dwBufLen);
		if (AdapterInfo == NULL) {
			printf("Error allocating memory needed to call GetAdaptersinfo\n");
		}
	}

	char tmp[256];
	set<string> macSet;

	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
		do {
			sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
				pAdapterInfo->Address[0], pAdapterInfo->Address[1],
				pAdapterInfo->Address[2], pAdapterInfo->Address[3],
				pAdapterInfo->Address[4], pAdapterInfo->Address[5]);

			if (!macSet.count(mac_addr) && !strstr(mac_addr, "00:50:56"))  // 忽略vmware的网卡
			{
				macSet.insert(mac_addr);
				macSet.insert(string(mac_addr).substr(0, 9));
				if (WIRELESS_TYPE == pAdapterInfo->Type && strstr(pAdapterInfo->Description, "802.11n"))
				{
					sprintf(tmp, " %s-%s", mac_addr, "wireless");
					mList.push_back(tmp);
				}
				else if (WIRED_TYPE == pAdapterInfo->Type)
				{
					sprintf(tmp, " %s-%s", mac_addr, "wired");
					mList.push_back(tmp);
				}
			}

			//            printf("mac: %s desc: %s,\n", mac_addr, pAdapterInfo->Description);

			pAdapterInfo = pAdapterInfo->Next;
		} while (pAdapterInfo);
	}
	free(AdapterInfo);

	return true;
}

// 获取本机有线网卡地址
bool GetWiredMac(string& wiredMac) 
{
	vector<string> mList;
	bool foundWired = false;


	if (!getMAC(mList))
	{
		// 获取网卡失败,这行代码不可能执行
		cout << "[ERROR] getMAC" << endl;
		return false;
	}

	int pos;
	for (size_t i = 0; i < mList.size(); i++)
	{
		cout << mList[i] << endl;
		pos = mList[i].find("-wired");

		// 获取有线网卡失败，则尝试获取无线网卡
		if (pos < 0)
		{
			pos = mList[i].find("-wireless");
		}

		if (pos > 0)
		{
			wiredMac = mList[i].substr(0, pos);
			foundWired = true;
			break;
		}
	}

	return foundWired;
}




// "MAC:2 11:22:33:44:55:66-wireless aa:bb:cc:dd:ee:ff-wired\n"
string FormatMAC() {
	vector<string> mList;
	string         txt = "MAC:";

	char           num[8] = { 0 };

	if (getMAC(mList))
	{
		sprintf(num, "%d", mList.size());
		txt += num;

		for (size_t i = 0; i < mList.size(); i++)
		{
			// cout << i << mList[i] << endl;
			txt += mList[i];
		}
	}
	txt += '\n';

	return txt;
}

bool CheckBuf(char *str)
{
	if (NULL == str)
	{
		return false;
	}

	//    cout << "GET " << str << endl;
	int strSize = strlen(str);
	for (; strSize >= 0; strSize--)
	{
		if (!isalnum(str[strSize]))
		{
			str[strSize] = 0;
		}
	}

	return strlen(str) > 5;
}

bool getHDS(vector<string>& mList)
{
	FILE*   fp = NULL;
	char    buf[256];
	char    tmp[256];

	const char* cmd = "wmic path win32_physicalmedia get SerialNumber";

	fp = _popen(cmd, "r");

	if (NULL == fp)
	{
		return false;
	}

	fgets(buf, 256, fp);
	memset(buf, 0, 256);
	while (NULL != fgets(buf, 256, fp))
	{
		if (CheckBuf(buf))
		{
			memset(tmp, 0, 256);
			sprintf(tmp, " %s-%s%d", buf, "HDS", mList.size() + 1);
			mList.push_back(tmp);
		}
		memset(buf, 0, 256);
	}

	_pclose(fp);

	return true;
}

string FormatHDS()
{
	vector<string> mList;
	string         txt = "HDS:";

	char           num[8] = { 0 };

	if (getHDS(mList))
	{
		sprintf(num, "%d", mList.size());
		txt += num;

		for (size_t i = 0; i < mList.size(); i++)
		{
			// cout << i << mList[i] << endl;
			txt += mList[i];
		}
	}
	txt += '\n';

	return txt;
}