// 头文件包含
#include "stdafx.h"
#include <WinSock2.h>
#include <Iphlpapi.h>
#include <iostream>
#include<string.h>

#pragma comment(lib,"iphlpapi.lib")

using namespace std;
// 函数声明
void output(PIP_ADAPTER_INFO pIpAdapterInfo);
struct MacInfomation
{
	BYTE MacAddress[128];//网卡地址
	int type;
};
struct MacStruct
{
	int sum;//一共多少个网卡

};
// 程序入口
int main(int argc, _TCHAR* argv[])
{
	//PIP_ADAPTER_INFO结构体指针存储本机网卡信息
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//得到结构体大小,用于GetAdaptersInfo参数
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//如果函数返回的是ERROR_BUFFER_OVERFLOW
		//则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
		//这也是说明为什么stSize既是一个输入量也是一个输出量
		//释放原来的内存空间
		delete pIpAdapterInfo;
		//重新申请内存空间用来存储所有网卡信息
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}


	if (ERROR_SUCCESS == nRel)
	{
		//输出网卡信息
		output(pIpAdapterInfo);
	}
	//释放内存空间
	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}
	return 0;
}
///函数作用,输出网卡信息
void output(PIP_ADAPTER_INFO pIpAdapterInfo)
{
	int sum = 0;
	int k = 0;
	//可能有多网卡,因此通过循环去判断
	enum NIC
	{
		wire = 1, wireless = 2, bluetooth = 3
	};
	MacInfomation macinformation[10];
	while (pIpAdapterInfo)
	{
		char *p1 = NULL;
		char *des = pIpAdapterInfo->Description;
		if ((p1 = strstr(des, "Wireless")) != NULL)
		{
		macinformation[k].type = 2;
		k++;
		p1 = NULL;
		}
		if ((p1 = strstr(des, "Virtual")) != NULL)
		{
		continue;
		p1 = NULL;
		}
		if ((p1 = strstr(des, "Bluetooth")) != NULL)
		{
		macinformation[k].type = 3;
		k++;
		p1 = NULL;
		}
		else
		{
		macinformation[k].type = 1;
		k++;
		p1 = NULL;
		}
		cout << "网卡描述：" << pIpAdapterInfo->Description << endl;
		cout << "网卡MAC地址：";
		for (UINT i = 0; i < pIpAdapterInfo->AddressLength; i++)
		{
			if (i == pIpAdapterInfo->AddressLength - 1)
			{
				printf("%02x\n", pIpAdapterInfo->Address[i]);
			}
			else
			{
				printf("%02x-", pIpAdapterInfo->Address[i]);
			}
		}

		pIpAdapterInfo = pIpAdapterInfo->Next;
		cout << "*****************************************************" << endl;
		sum++;
	}
	cout << "网卡的总数是" << sum << endl;
	return;
}
