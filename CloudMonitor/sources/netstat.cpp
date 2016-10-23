// Need to link with Iphlpapi.lib and Ws2_32.lib
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include "NetMon.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

int GetConnections(vector<Connection> &cons)
{
	// Declare and initialize variables
	PMIB_TCPTABLE pTcpTable;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;
	Connection cur;

	struct in_addr IpAddr;

	int i;

	pTcpTable = (MIB_TCPTABLE *)MALLOC(sizeof(MIB_TCPTABLE));
	if (pTcpTable == NULL) {
		printf("Error allocating memory\n");
		return 1;
	}

	dwSize = sizeof(MIB_TCPTABLE);
	// Make an initial call to GetTcpTable to
	// get the necessary size into the dwSize variable
	if ((dwRetVal = GetTcpTable(pTcpTable, &dwSize, TRUE)) ==
		ERROR_INSUFFICIENT_BUFFER) {
		FREE(pTcpTable);
		pTcpTable = (MIB_TCPTABLE *)MALLOC(dwSize);
		if (pTcpTable == NULL) {
			printf("Error allocating memory\n");
			return 1;
		}
	}
	// Make a second call to GetTcpTable to get
	// the actual data we require
	if ((dwRetVal = GetTcpTable(pTcpTable, &dwSize, TRUE)) == NO_ERROR) {
		//printf("\tNumber of entries: %d\n", (int)pTcpTable->dwNumEntries);
		for (i = 0; i < (int)pTcpTable->dwNumEntries; i++) {
			memset(&cur, 0, sizeof(cur));
			IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwLocalAddr;
			//strcpy_s(szLocalAddr, sizeof(szLocalAddr), inet_ntoa(IpAddr));
			strcpy(cur.local, inet_ntoa(IpAddr));
			IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwRemoteAddr;
			strcpy(cur.remote,  inet_ntoa(IpAddr));
			cur.state = pTcpTable->table[i].dwState;

			cur.lport = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
			cur.rport = ntohs((u_short)pTcpTable->table[i].dwRemotePort);
			cur.report = FALSE;
			cons.push_back(cur);
		}
	}
	else {
		printf("\tGetTcpTable failed with %d\n", dwRetVal);
		FREE(pTcpTable);
		return 1;
	}

	if (pTcpTable != NULL) {
		FREE(pTcpTable);
		pTcpTable = NULL;
	}
	return 0;
}

int NetStat()
{

	// Declare and initialize variables
	PMIB_TCPTABLE pTcpTable;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;

	char szLocalAddr[128];
	char szRemoteAddr[128];

	struct in_addr IpAddr;

	int i;

	pTcpTable = (MIB_TCPTABLE *)MALLOC(sizeof(MIB_TCPTABLE));
	if (pTcpTable == NULL) {
		printf("Error allocating memory\n");
		return 1;
	}

	dwSize = sizeof(MIB_TCPTABLE);
	// Make an initial call to GetTcpTable to
	// get the necessary size into the dwSize variable
	if ((dwRetVal = GetTcpTable(pTcpTable, &dwSize, TRUE)) ==
		ERROR_INSUFFICIENT_BUFFER) {
		FREE(pTcpTable);
		pTcpTable = (MIB_TCPTABLE *)MALLOC(dwSize);
		if (pTcpTable == NULL) {
			printf("Error allocating memory\n");
			return 1;
		}
	}
	// Make a second call to GetTcpTable to get
	// the actual data we require
	if ((dwRetVal = GetTcpTable(pTcpTable, &dwSize, TRUE)) == NO_ERROR) {
		printf("\tNumber of entries: %d\n", (int)pTcpTable->dwNumEntries);
		for (i = 0; i < (int)pTcpTable->dwNumEntries; i++) {
			IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwLocalAddr;
			strcpy_s(szLocalAddr, sizeof(szLocalAddr), inet_ntoa(IpAddr));
			IpAddr.S_un.S_addr = (u_long)pTcpTable->table[i].dwRemoteAddr;
			strcpy_s(szRemoteAddr, sizeof(szRemoteAddr), inet_ntoa(IpAddr));

			printf("\n\tTCP[%d] State: %ld - ", i,
				pTcpTable->table[i].dwState);
			switch (pTcpTable->table[i].dwState) {
			case MIB_TCP_STATE_CLOSED:
				printf("CLOSED\n");
				break;
			case MIB_TCP_STATE_LISTEN:
				printf("LISTEN\n");
				break;
			case MIB_TCP_STATE_SYN_SENT:
				printf("SYN-SENT\n");
				break;
			case MIB_TCP_STATE_SYN_RCVD:
				printf("SYN-RECEIVED\n");
				break;
			case MIB_TCP_STATE_ESTAB:
				printf("ESTABLISHED\n");
				break;
			case MIB_TCP_STATE_FIN_WAIT1:
				printf("FIN-WAIT-1\n");
				break;
			case MIB_TCP_STATE_FIN_WAIT2:
				printf("FIN-WAIT-2 \n");
				break;
			case MIB_TCP_STATE_CLOSE_WAIT:
				printf("CLOSE-WAIT\n");
				break;
			case MIB_TCP_STATE_CLOSING:
				printf("CLOSING\n");
				break;
			case MIB_TCP_STATE_LAST_ACK:
				printf("LAST-ACK\n");
				break;
			case MIB_TCP_STATE_TIME_WAIT:
				printf("TIME-WAIT\n");
				break;
			case MIB_TCP_STATE_DELETE_TCB:
				printf("DELETE-TCB\n");
				break;
			default:
				printf("UNKNOWN dwState value\n");
				break;
			}
			printf("\tTCP[%d] Local Addr: %s\n", i, szLocalAddr);
			printf("\tTCP[%d] Local Port: %d \n", i,
				ntohs((u_short)pTcpTable->table[i].dwLocalPort));
			printf("\tTCP[%d] Remote Addr: %s\n", i, szRemoteAddr);
			printf("\tTCP[%d] Remote Port: %d\n", i,
				ntohs((u_short)pTcpTable->table[i].dwRemotePort));
		}
	}
	else {
		printf("\tGetTcpTable failed with %d\n", dwRetVal);
		FREE(pTcpTable);
		return 1;
	}

	if (pTcpTable != NULL) {
		FREE(pTcpTable);
		pTcpTable = NULL;
	}
	return 0;
}


void ViewConnections(vector<Connection> &cons)
{
	for (DWORD i = 0; i < cons.size(); i++)
	{
		printf("[%d] %d %-16s%-8d  %-16s%-8d", i, cons[i].report, cons[i].local, cons[i].lport, cons[i].remote, cons[i].rport);
		switch (cons[i].state) {
		case MIB_TCP_STATE_CLOSED:
			printf("CLOSED\n");
			break;
		case MIB_TCP_STATE_LISTEN:
			printf("LISTEN\n");
			break;
		case MIB_TCP_STATE_SYN_SENT:
			printf("SYN-SENT\n");
			break;
		case MIB_TCP_STATE_SYN_RCVD:
			printf("SYN-RECEIVED\n");
			break;
		case MIB_TCP_STATE_ESTAB:
			printf("ESTABLISHED\n");
			break;
		case MIB_TCP_STATE_FIN_WAIT1:
			printf("FIN-WAIT-1\n");
			break;
		case MIB_TCP_STATE_FIN_WAIT2:
			printf("FIN-WAIT-2 \n");
			break;
		case MIB_TCP_STATE_CLOSE_WAIT:
			printf("CLOSE-WAIT\n");
			break;
		case MIB_TCP_STATE_CLOSING:
			printf("CLOSING\n");
			break;
		case MIB_TCP_STATE_LAST_ACK:
			printf("LAST-ACK\n");
			break;
		case MIB_TCP_STATE_TIME_WAIT:
			printf("TIME-WAIT\n");
			break;
		case MIB_TCP_STATE_DELETE_TCB:
			printf("DELETE-TCB\n");
			break;
		default:
			printf("UNKNOWN dwState value\n");
			break;
		}
	}
}


void DisplayPorts(vector<Service> &KeyPorts)
{
	for (unsigned int i = 0; i < KeyPorts.size(); i++)
	{
		KeyPorts[i].show();
	}
}

int ReadKeyPorts(char *FileName, vector<Service> &KeyPorts)
{
	char         CurLine[MAX_LINE];
	char         desc[MAX_LINE];
	char         service[MAX_LINE];
	FILE *fp;

	if ((fp = fopen(FileName, "r")) == NULL)
	{
		perror(FileName);
		return -1;
	}

	char *tp;
	int port;

	while (fgets(CurLine, 256, fp))
	{
		memset(service, 0, sizeof(service));
		memset(desc, 0, sizeof(desc));
		port = 0;
		sscanf(CurLine, "%s %d ", service, &port);
		if ((tp = strchr(CurLine, '#')) != NULL)
		{
			strcat(desc, tp + 2);
		}
		else
		{
			desc[0] = '\n';
		}
		//printf("%-16s %-8d %s", service, port, desc);
		KeyPorts.push_back(Service(port, service, desc));
	}
	fclose(fp);

	return 0;
}


int CheckKeyConnections(vector<Connection> &cons, vector<Service> &KeyPorts)
{
	int cnt = 0;


	for (DWORD i = 0; i < cons.size(); i++)
	{
		for (DWORD j = 0; j < KeyPorts.size(); j++)
		{
			if (KeyPorts[j].IsYou(cons[i].rport, cons[i].desc))
			{
				cons[i].report = TRUE;
				cnt += 1;
				//printf("matched: ");
				//printf("[%d] %-16s%-8d  %-16s%-8d\n", i, cons[i].local, cons[i].lport, cons[i].remote, cons[i].rport);
			}
		}
	}

	return cnt;
}

int ShowKeyConnections(vector<Connection> &cons, int size)
{
	int cnt = 0;

	printf("ShowKeyConnections\n");
	for (DWORD i = 0; i < cons.size() && cnt < size; i++)
	{
		if (TRUE == cons[i].report)
		{
			//cout << "description: " << cons[i].desc;
			printf("[%d] %-16s%6d  %-16s%-6d %s", cnt, cons[i].local, cons[i].lport, cons[i].remote, cons[i].rport, cons[i].desc);
			cnt += 1;
		}
	}

	return cnt;
}