// ͷ�ļ�����
#include "stdafx.h"
#include <WinSock2.h>
#include <Iphlpapi.h>
#include <iostream>
#include<string.h>

#pragma comment(lib,"iphlpapi.lib")

using namespace std;
// ��������
void output(PIP_ADAPTER_INFO pIpAdapterInfo);
struct MacInfomation
{
	BYTE MacAddress[128];//������ַ
	int type;
};
struct MacStruct
{
	int sum;//һ�����ٸ�����

};
// �������
int main(int argc, _TCHAR* argv[])
{
	//PIP_ADAPTER_INFO�ṹ��ָ��洢����������Ϣ
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//�õ��ṹ���С,����GetAdaptersInfo����
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//����GetAdaptersInfo����,���pIpAdapterInfoָ�����;����stSize��������һ��������Ҳ��һ�������
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//����������ص���ERROR_BUFFER_OVERFLOW
		//��˵��GetAdaptersInfo�������ݵ��ڴ�ռ䲻��,ͬʱ�䴫��stSize,��ʾ��Ҫ�Ŀռ��С
		//��Ҳ��˵��ΪʲôstSize����һ��������Ҳ��һ�������
		//�ͷ�ԭ�����ڴ�ռ�
		delete pIpAdapterInfo;
		//���������ڴ�ռ������洢����������Ϣ
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//�ٴε���GetAdaptersInfo����,���pIpAdapterInfoָ�����
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}


	if (ERROR_SUCCESS == nRel)
	{
		//���������Ϣ
		output(pIpAdapterInfo);
	}
	//�ͷ��ڴ�ռ�
	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}
	return 0;
}
///��������,���������Ϣ
void output(PIP_ADAPTER_INFO pIpAdapterInfo)
{
	int sum = 0;
	int k = 0;
	//�����ж�����,���ͨ��ѭ��ȥ�ж�
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
		cout << "����������" << pIpAdapterInfo->Description << endl;
		cout << "����MAC��ַ��";
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
	cout << "������������" << sum << endl;
	return;
}
