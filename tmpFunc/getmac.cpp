// 头文件包含
#include "stdafx.h"

#pragma comment(lib,"iphlpapi.lib")

using namespace std;
// 函数声明
struct MacStruct;
MacStruct output(PIP_ADAPTER_INFO pIpAdapterInfo);


typedef struct MacInfo
{
	BYTE MacAddress[6];//网卡地址
	char Desc[128];//网卡的描述 是结构体中的type具体表达形式
	int type;
	MacInfo(const MacInfo& mi) {
		this->type = mi.type;
		memcpy(this->MacAddress, mi.MacAddress, sizeof(MacAddress));
		memcpy(this->Desc, mi.Desc, sizeof(Desc));
	}
} MacInfo;


struct MacStruct
{
	int sum;//一共多少个网卡
	MacInfo *macInfomation;
	MacStruct(int n, MacInfo* ptr) : sum(n), macInfomation(ptr) {}
	//MacStruct():sum(0),macInfomation(0) {}

	MacStruct(const MacStruct& a) :sum(0), macInfomation(0) { this->sum = a.sum; this->macInfomation = a.macInfomation; }
};


// 程序入口
int call();


//int main(int argc, _TCHAR* argv[])
//{
//	call();//调用函数
//	return 0;
//}



int call()
{
	//PIP_ADAPTER_INFO结构体指针存储本机网卡信息
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//得到结构体大小,用于GetAdaptersInfo参数
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	while (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//如果函数返回的是ERROR_BUFFER_OVERFLOW
		//则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
		//这也是说明为什么stSize既是一个输入量也是一个输出量
		//释放原来的内存空间
		stSize += sizeof(IP_ADAPTER_INFO);
		delete pIpAdapterInfo;
		//重新申请内存空间用来存储所有网卡信息
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}

	if (ERROR_SUCCESS == nRel)
	{
		//输出网卡信息
		MacStruct result(output(pIpAdapterInfo));
		
		cout << result.sum << endl;
		//cout << (void*)result.macInfomation << result.macInfomation->Desc << endl;
		for (int i = 0; i < result.sum; i++) {
		cout << result.macInfomation[i].type << endl;
		cout << result.macInfomation[i].Desc << endl;
		for (int j = 0; j < 6; j++) {
		printf("%02x-", result.macInfomation[i].MacAddress[j]);
		}
		cout << endl;
		}
		free(result.macInfomation);
		
	}

	//释放内存空间
	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
		pIpAdapterInfo = 0;
	}
	return 0;
}
///函数作用,输出网卡信息
MacStruct output(PIP_ADAPTER_INFO pIpAdapterInfo)
{
	int sum = 0;
	int k = 0;
	//可能有多网卡,因此通过循环去判断
	enum NIC
	{
		wire = 1, wireless = 2, bluetooth = 3
	};
	MacInfo* macinformation = 0;
	while (pIpAdapterInfo)
	{
		char *des = pIpAdapterInfo->Description;
		if (strstr(des, "Wireless") != NULL)
		{

			macinformation = (MacInfo*)realloc(macinformation, (k + 1)*(sizeof(MacInfo)));
			macinformation[k].type = wireless;
			strcpy(macinformation[k].Desc, des);
			for (UINT i = 0; i < pIpAdapterInfo->AddressLength; i++) {
				macinformation[k].MacAddress[i] = pIpAdapterInfo->Address[i];
			}
			k++;

		}
		else if (strstr(des, "Virtual") != NULL)
		{
			pIpAdapterInfo = pIpAdapterInfo->Next;
			continue;
		}
		else if (strstr(des, "Bluetooth") != NULL)
		{
			macinformation = (MacInfo*)realloc(macinformation, (1 + k)*(sizeof(MacInfo)));
			macinformation[k].type = bluetooth;
			strcpy(macinformation[k].Desc, des);
			for (UINT i = 0; i < pIpAdapterInfo->AddressLength; i++) {
				macinformation[k].MacAddress[i] = pIpAdapterInfo->Address[i];
			}
			k++;
		}
		else
		{
			macinformation = (MacInfo*)realloc(macinformation, (1 + k)*(sizeof(MacInfo)));
			macinformation[k].type = 1;
			strcpy(macinformation[k].Desc, des);
			for (UINT i = 0; i < pIpAdapterInfo->AddressLength; i++) {
				macinformation[k].MacAddress[i] = pIpAdapterInfo->Address[i];
			}
			k++;
		}

		pIpAdapterInfo = pIpAdapterInfo->Next;
	}
	return MacStruct(k, macinformation);
}
