#include "mUSB.h"

#include <iostream>
#include <set>
#include <map>


#define BUFSIZE 1024

BOOL GetUSBDriverInfo(LPSTR szDrive, USB &usb);


BOOL EnumDevices(USB &usb)
{
	CHAR szLogicDriveStrings[BUFSIZE];
	PCHAR szDrive;
	BOOL usbPluged = FALSE;

	ZeroMemory(szLogicDriveStrings, BUFSIZE);

	GetLogicalDriveStrings(BUFSIZE - 1, szLogicDriveStrings);
	szDrive = (PCHAR)szLogicDriveStrings;

	do
	{
		if (GetUSBDriverInfo(szDrive, usb))
		{
			usbPluged = TRUE;
		}
		szDrive += (lstrlen(szDrive) + 1);
	} while (*szDrive != '\x00');

	//system("PAUSE");
	return usbPluged;
}


BOOL GetUSBDriverInfo(LPSTR szDrive, USB &usb)
{
	UINT uDriverType;
	DWORD dwVolumeSerialNumber;
	DWORD dwMaximumComponentlength;
	DWORD dwFileSystemFlags;
	CHAR szFileSystemNameBuffer[BUFSIZE];
	CHAR szDriveName[MAX_PATH];

	uDriverType = GetDriveType(szDrive);

	BOOL isRemovable = FALSE;

	switch (uDriverType)
	{
	case DRIVE_FIXED:
		//printf("[LOCAL DEVICE] ");
		break;
	case DRIVE_REMOVABLE:
		isRemovable = TRUE;
		//printf("\t[USB DEVICE] ");
		break;
	case DRIVE_REMOTE:
		isRemovable = FALSE;
		//printf("\t[NETWORK DEVICE] ");
		break;
	case DRIVE_CDROM:
		isRemovable = FALSE;
		//printf("\t[CD-ROM] ");
		break;
	default:
		break;
	}

	// ������ǿɲ���豸����ֱ�ӷ���
	if (!isRemovable)
	{
		return isRemovable;
	}

	//printf("%s\n", szDrive);
	usb.setDevMount(szDrive[0]);
	if (!(GetVolumeInformation(
		szDrive,
		szDriveName,
		MAX_PATH,
		&dwVolumeSerialNumber,
		&dwMaximumComponentlength,
		&dwFileSystemFlags,
		szFileSystemNameBuffer,
		BUFSIZE)))
	{
		return isRemovable;

	}

	if (0 != lstrlen(szDriveName))
	{

		//strcpy_s(usb.devName, szDriveName);
		usb.setDevName(szDriveName);
		// printf("\t[Drive Name] %s\n", szDriveName);
	}
	else
	{
		//strcpy_s(usb.devName, "���ƶ�����");
		usb.setDevName("���ƶ�����");
		//printf("\t[Drive Name] %s\n", "���ƶ�����");
	}

	//sprintf_s(usb.devSeri, "%u", dwVolumeSerialNumber);
	usb.setDevSerial(dwVolumeSerialNumber);
	//printf("\t[Serial Num] %u\n", dwVolumeSerialNumber);

	//printf("\n\n");

	return isRemovable;

}


BOOL CheckUsbDevice(USB &usb)
{
	static BOOL    usbOnceIn = FALSE;	// ��¼U���Ƿ����������
	static map<char, bool> lastStatu;	// ��¼U�̵���һ��״̬

	BOOL	usbPluged = FALSE;
	
	usbPluged = EnumDevices(usb);

	// ֮����ʹ�� '1 == usbPluged' 
	// ��Ϊ��ȡ�� vs2015 ��BOOL �� bool ������ת���ľ���
	// û�취��΢��ϲ����д�������� DWORD, SIZE_T, CHAR
	// ����������ԭ��̬��Сд������: int, bool, char
	usb.updateStatus(1 == usbPluged);

	if (usbPluged)
	{
		usbOnceIn = TRUE;
		//statuList[usb.getDevMount()] = usb.getStatu();
	}

	// ��δ�����U��
	if (!usbOnceIn)
	{
		//printf("  NO USB DEVICES\n\n");
		return FALSE;
	}
	if (lastStatu[usb.getDevMount()] != usb.getStatu())
	{
		lastStatu[usb.getDevMount()] = usb.getStatu();
		return TRUE;
	}

	return FALSE;
}


// ����demo
#if 0

int main()
{
	DWORD	count = 0;
	string	message;
	USB		usb;

	string tmp;

	int wait = 1000;

	while (TRUE)
	{
		count += 1;
		Sleep(wait);
		system("cls");
		printf("LOOP[%d]\n", count);
	
		if (CheckUsbDevice(usb))
		{
			// ��������״̬�ı�ʱ����ӡ֪ͨ
			wait *= 3;
			cout << usb.getMessage() << endl;
		}
		else
		{
			wait = 1000;
		}

	}
	
	return 0;
}
#endif