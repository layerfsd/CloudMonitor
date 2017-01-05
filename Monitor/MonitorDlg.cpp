
// MonitorDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Monitor.h"
#include "MonitorDlg.h"
#include "afxdialogex.h"

#pragma comment(lib,"ws2_32.lib")

//#include <vector>
//#include <map>
//
//using namespace;
//
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMonitorDlg �Ի���



CMonitorDlg::CMonitorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MONITOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_USERNAME, username);
	DDX_Control(pDX, IDC_PASSWD, passwd);
	DDX_Control(pDX, IDC_STATUS, inform);
}

BEGIN_MESSAGE_MAP(CMonitorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMonitorDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_USERNAME, &CMonitorDlg::OnEnChangeUsername)
	ON_STN_CLICKED(IDC_STATUS, &CMonitorDlg::OnStnClickedStatus)
	ON_BN_CLICKED(IDCANCEL, &CMonitorDlg::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_PASSWD, &CMonitorDlg::OnEnChangePasswd)
END_MESSAGE_MAP()


// CMonitorDlg ��Ϣ�������

BOOL CMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	//ShowWindow(SW_MINIMIZE);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMonitorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMonitorDlg::OnNMCustomdrawProgress1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}


SOCKET slisten;
int Talk2Client()
{
	SOCKET sClient;
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	char revData[32];

	memset(revData, 0, sizeof(revData));
	sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);

	//��������
	recv(sClient, revData, sizeof(revData)-1, 0);

	closesocket(sClient);
	closesocket(slisten);
	
	WSACleanup();
	return atoi(revData);
}

bool CreateTCPServer()
{
	//��ʼ��WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return false;
	}

	//�����׽���
	slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket error !");
		return false;
	}

	//��IP�Ͷ˿�
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(159);
	inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
	//sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
		return false;
	}

	//��ʼ����
	if (listen(slisten, 1) == SOCKET_ERROR)
	{
		printf("listen error !");
		return false;
	}

	return true;
}

static int  albSockRet;

DWORD WINAPI Func(void* pArg)
{
	albSockRet = Talk2Client();
	return 0;
}

static int IsCnt2Internet()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKADDR_IN addrSrv;
	int err;
	int ret;
	char m_ipaddr[16] = "121.42.146.43";


	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

	unsigned long NonBlock = 1;

	int ReceiveTimeout = 1500;
	setsockopt(sockClient, SOL_SOCKET, SO_RCVTIMEO, (char*)&ReceiveTimeout, sizeof(int));

	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(80);

	inet_pton(AF_INET, m_ipaddr, &addrSrv.sin_addr);
	ret = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));


	closesocket(sockClient);
	WSACleanup();
	return ret;
}


void CMonitorDlg::OnBnClickedOk()
{
	
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	SetDlgItemText(IDC_STATUS, L" ");

	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString			s_name, s_pass;
	//ALB_SOCK_RET	ret;
	CString			inform;
	UpdateData(TRUE);

	username.GetWindowText(s_name);
	passwd.GetWindowText(s_pass);

	if (s_name.GetLength() <= 0)
	{
		inform = "�û�������Ϊ��";
		//SetDlgItemText(IDC_STATUS, inform);
		AfxMessageBox(inform);
		return;
	}

	if (s_pass.GetLength() <= 0)
	{
		inform = "���벻��Ϊ��";
		//SetDlgItemText(IDC_STATUS, inform);
		AfxMessageBox(inform);
		return;
	}
	char cWinDir[MAX_PATH];
	char cmd[MAX_PATH];
	const char *cname;
	const char *cpass;

	_bstr_t fuckName(s_name.GetBuffer());
	cname = fuckName;

	_bstr_t fuckPass(s_pass);
	cpass = fuckPass;


	if (strchr(cpass, ' '))
	{

		inform = "�����в�������ڿո�";
		//SetDlgItemText(IDC_STATUS, inform);
		AfxMessageBox(inform);
		return;
	}

	if (0 != IsCnt2Internet())
	{
		inform = "��������״���쳣";
		//SetDlgItemText(IDC_STATUS, inform);
		SetDlgItemText(IDC_STATUS, NULL);
		AfxMessageBox(inform);
		return;
	}


	memset(cmd, 0, MAX_PATH);
	GetCurrentDirectoryA(MAX_PATH, cWinDir);

	char* sAppPath = cWinDir;
	strcat_s(cWinDir, sizeof(cWinDir), "\\CloudMonitor.exe");

	sprintf_s(cmd, MAX_PATH, "%s %s %s", sAppPath, cname, cpass);


	STARTUPINFOA   StartupInfo;//���������������Ϣ�ṹ����    
	PROCESS_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);


	// �ں�̨���� CloudMonitor.exe ����
	if (CreateProcessA(NULL,
		cmd,
		NULL, 
		NULL, 
		FALSE, 
		//0,
		CREATE_NO_WINDOW,
		NULL,
		NULL, 
		&StartupInfo,
		&pi))
	{

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		inform = "���ڵ�¼ ...";
		SetDlgItemText(IDC_STATUS, inform);

	}
	else
	{
		inform = sAppPath;
		inform += "��½ʧ��";
		SetDlgItemText(IDC_STATUS, NULL);
		AfxMessageBox(inform);
		CDialogEx::OnOK();

	}


	DWORD dwRet = 0;

	if (CreateTCPServer())
	{
		inform = "���ڴ�Զ�̶˿� ...";
		SetDlgItemText(IDC_STATUS, inform);

	     HANDLE hThread = CreateThread(NULL, 0, Func, 0, NULL, NULL);//���������߳�

		 bool proceccFlag = false;
		do
		{

			dwRet = ::MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT);
			DoEvent();
			
			if (proceccFlag != true && NONSENSE == albSockRet)
			{
				proceccFlag = true;
				inform = "������֤�û��������� ...";
				SetDlgItemText(IDC_STATUS, inform);
				//GetDlgItem(IDC_PASSWD)->SetWindowTextW(L"");
			}

			if (CONNECT_FAILED == albSockRet)
			{
				inform = "���ӷ�����ʧ��";
				//SetDlgItemText(IDC_STATUS, inform);
				SetDlgItemText(IDC_STATUS, NULL);
				AfxMessageBox(inform);
				break;
			}
			if (NOT_SPECIFIC_MAC == albSockRet)
			{
				//inform = "��ǰ�û��������������ļ�����ϵ�¼";
				inform = "��ǰ��¼��ʹ�õ��û����Ѿ����������԰󶨣������Ҫ���°󶨵�ǰ���ԣ����� '���°�'��ť";
				//SetDlgItemText(IDC_STATUS, inform);
				SetDlgItemText(IDC_STATUS, NULL);
				AfxMessageBox(inform);
				break;
			}
			if (INVALID_PASSWD == albSockRet)
			{
				inform = "�û������������";
				//SetDlgItemText(IDC_STATUS, inform);
				GetDlgItem(IDC_PASSWD)->SetWindowTextW(L"");
				SetDlgItemText(IDC_STATUS, NULL);
				AfxMessageBox(inform);
				break;
			}
			if (ALREADY_LOGIN == albSockRet)
			{
				inform = "���ѵ�¼����,����Ҫ�ظ���¼��һ̨����ֻ�����¼һ���û���";
				//SetDlgItemText(IDC_STATUS, inform);
				SetDlgItemText(IDC_STATUS, NULL);
				AfxMessageBox(inform);
				break;
			}
		} while ((dwRet != WAIT_OBJECT_0) && (dwRet != WAIT_FAILED));
		CloseHandle(hThread);
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	else
	{
		inform = "��������tcpͨ��ʧ��";
		//SetDlgItemText(IDC_STATUS, inform);
		SetDlgItemText(IDC_STATUS, NULL);
		AfxMessageBox(inform);
	}

	if (ALREADY_LOGIN == albSockRet)
	{
		CDialogEx::OnOK();
	}
	if (CONNECT_SUCCESS == albSockRet)
	{
		inform = "��¼�ɹ�";
		//SetDlgItemText(IDC_STATUS, inform);
		SetDlgItemText(IDC_STATUS, NULL);
		AfxMessageBox(inform);
		CDialogEx::OnOK();
	}
	albSockRet = NONSENSE;
}


void CMonitorDlg::OnEnChangeUsername()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDOK)->EnableWindow(TRUE);

}


void CMonitorDlg::OnStnClickedStatus()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CMonitorDlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnCancel();
}


void CMonitorDlg::OnEnChangePasswd()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDOK)->EnableWindow(TRUE);
}


void CMonitorDlg::DoEvent()
{
	MSG msg;
	if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))  //ȡ��Ϣ������Ӧ�ó������Ϣ���У�PM_REMOVEȡ��֮�����Ϣ�������Ƴ�  
	{
		//����Ϣ  
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}