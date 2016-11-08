// ͷ�ļ�����
#include "stdafx.h"

#pragma comment(lib,"iphlpapi.lib")

using namespace std;
// ��������
struct MacStruct;
MacStruct output(PIP_ADAPTER_INFO pIpAdapterInfo);


typedef struct MacInfo
{
	BYTE MacAddress[6];//������ַ
	char Desc[128];//���������� �ǽṹ���е�type��������ʽ
	int type;
	MacInfo(const MacInfo& mi) {
		this->type = mi.type;
		memcpy(this->MacAddress, mi.MacAddress, sizeof(MacAddress));
		memcpy(this->Desc, mi.Desc, sizeof(Desc));
	}
} MacInfo;


struct MacStruct
{
	int sum;//һ�����ٸ�����
	MacInfo *macInfomation;
	MacStruct(int n, MacInfo* ptr) : sum(n), macInfomation(ptr) {}
	//MacStruct():sum(0),macInfomation(0) {}

	MacStruct(const MacStruct& a) :sum(0), macInfomation(0) { this->sum = a.sum; this->macInfomation = a.macInfomation; }
};


// �������
int call();


//int main(int argc, _TCHAR* argv[])
//{
//	call();//���ú���
//	return 0;
//}



int call()
{
	//PIP_ADAPTER_INFO�ṹ��ָ��洢����������Ϣ
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	//�õ��ṹ���С,����GetAdaptersInfo����
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	//����GetAdaptersInfo����,���pIpAdapterInfoָ�����;����stSize��������һ��������Ҳ��һ�������
	int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	while (ERROR_BUFFER_OVERFLOW == nRel)
	{
		//����������ص���ERROR_BUFFER_OVERFLOW
		//��˵��GetAdaptersInfo�������ݵ��ڴ�ռ䲻��,ͬʱ�䴫��stSize,��ʾ��Ҫ�Ŀռ��С
		//��Ҳ��˵��ΪʲôstSize����һ��������Ҳ��һ�������
		//�ͷ�ԭ�����ڴ�ռ�
		stSize += sizeof(IP_ADAPTER_INFO);
		delete pIpAdapterInfo;
		//���������ڴ�ռ������洢����������Ϣ
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		//�ٴε���GetAdaptersInfo����,���pIpAdapterInfoָ�����
		nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
	}

	if (ERROR_SUCCESS == nRel)
	{
		//���������Ϣ
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

	//�ͷ��ڴ�ռ�
	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
		pIpAdapterInfo = 0;
	}
	return 0;
}
///��������,���������Ϣ
MacStruct output(PIP_ADAPTER_INFO pIpAdapterInfo)
{
	int sum = 0;
	int k = 0;
	//�����ж�����,���ͨ��ѭ��ȥ�ж�
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
