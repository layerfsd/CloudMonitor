#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <Windows.h>

#include <string>

using namespace std;


static const char *status[] = {
	"USB Plugedged OUT",
	"USB Plugedged IN",
};
class USB
{
public:
	USB()
	{
		memset(this->devName, 0, 32);
		memset(this->devSeri, 0, 32);
		this->devMout = 0;
		isPlug = false;
	}

	void updateStatus(bool status = false)
	{
		this->isPlug = status;
	}

	void setDevName(char *name)
	{
		strcpy_s(this->devName, 32, name);
	}

	void setDevSerial(DWORD serial)
	{
		sprintf_s(this->devSeri, "%u", serial);
	}

	void setDevMount(char mount)
	{
		this->devMout = mount;
	}

	char getDevMount()
	{
		return this->devMout;
	}

	bool getStatu()
	{
		return this->isPlug;
	}

	string getMessage()
	{
		// 记录usb状态(plug [in|out])
		message = status[this->isPlug];
		message += '\n';


		// 记录usb参数
		message += "usb name:";
		message += this->devName;
		message += "|";

		message += "serial number:";
		message += this->devSeri;
		message += "|";

		message += "mount drive:";
		message += this->devMout;

		return message;
	}
private:
	string message;
	char devName[32];
	char devSeri[32];
	char devMout;
	bool isPlug;
};

