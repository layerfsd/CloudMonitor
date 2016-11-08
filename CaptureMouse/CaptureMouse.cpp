// CaptureMouse.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


extern "C" VOID SetHookOn();
extern "C" VOID SetHookOff();
		
#pragma comment (lib, "HooKeyboard.lib")

int main()
{
	bool status = false;
	char input[16];

	while (true)
	{
		std::cout << "hook state: " << status << std::endl;
		std::cin >> input;
		status = !status;
		if (status)
		{
			SetHookOn();
		}
		else
		{
			SetHookOff();
		}
		std::cout << "hook state: " << status << std::endl;
	} 

	return 0;
}