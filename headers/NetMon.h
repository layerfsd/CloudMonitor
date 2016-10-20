#pragma once
#include "network.h"

#include <iostream>
#include <cstdio>
#include <Windows.h>
#include <vector>
#include <string>

using namespace std;

#ifndef _NETMON_H__
#define _NETMON_H__

#define PORT_STATUS_SIZE 16
#define MAX_LINE		 256
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
/* Note: could also use malloc() and free() */


struct Connection
{
	char	local[IP_SIZE];
	char	remote[IP_SIZE];
	char	desc[MAX_LINE];
	WORD	lport;
	WORD	rport;
	DWORD   state;
	BOOL    report;
};

class Service
{
public:
	Service()
	{
		port = 0;
	}
	Service(int port, char *service, char* desc)
	{
		this->port = port;
		this->service = service;
		this->desc = desc;
	}

	void show()
	{
		printf("%-16s %-8d %s", service.c_str(), port, desc.c_str());
	}

	int IsYou(int p, char *desc)
	{
		if (p == this->port)
		{
			strncpy(desc, this->desc.c_str(), MAX_LINE -1);
			return TRUE;
		}
		return FALSE;
	}

private:
	int	    port;
	string  desc;
	string  service;
};


int GetMac(char *mac_addr);
int GetConnections(vector<Connection> &cons);
void ViewConnections(vector<Connection> &cons);
void DisplayPorts(vector<Service> &KeyPorts);
int ReadKeyPorts(char *FileName, vector<Service> &KeyPorts);
int CheckKeyConnections(vector<Connection> &cons, vector<Service> &KeyPorts);
int ShowKeyConnections(vector<Connection> &cons, int size);

int NetStat();

#endif // !_NETMON_H__
