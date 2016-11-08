// MonitroService.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <signal.h>


typedef void(*SignalHandlerPointer)(int);

void SignalHandler(int signal)
{
	printf("\nExciting...\n");
	SetHookOff();
	//exit(signal);
}

int main()
{
	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGINT, SignalHandler);
	SetHookOn();	
	printf("end\n");
	return 0;
}

