
// MonitorDlg.cpp : 实现文件
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


// CMonitorDlg 对话框



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


// CMonitorDlg 消息处理程序

BOOL CMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMonitorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMonitorDlg::OnNMCustomdrawProgress1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
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

	//接收数据
	recv(sClient, revData, sizeof(revData)-1, 0);

	closesocket(sClient);
	closesocket(slisten);
	
	WSACleanup();
	return atoi(revData);
}

bool CreateTCPServer()
{
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return false;
	}

	//创建套接字
	slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket error !");
		return false;
	}

	//绑定IP和端口
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

	//开始监听
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

	// TODO: 在此添加控件通知处理程序代码
	CString			s_name, s_pass;
	//ALB_SOCK_RET	ret;
	CString			inform;
	UpdateData(TRUE);

	username.GetWindowText(s_name);
	passwd.GetWindowText(s_pass);

	if (s_name.GetLength() <= 0)
	{
		inform = "用户名不能为空";
		//SetDlgItemText(IDC_STATUS, inform);
		AfxMessageBox(inform);
		return;
	}

	if (s_pass.GetLength() <= 0)
	{
		inform = "密码不能为空";
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

		inform = "密码中不允许存在空格";
		//SetDlgItemText(IDC_STATUS, inform);
		AfxMessageBox(inform);
		return;
	}

	if (0 != IsCnt2Internet())
	{
		inform = "您的网络状况异常";
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


	STARTUPINFOA   StartupInfo;//创建进程所需的信息结构变量    
	PROCESS_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);


	// 在后台开启 CloudMonitor.exe 进程
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
		inform = "正在登录 ...";
		SetDlgItemText(IDC_STATUS, inform);

	}
	else
	{
		inform = sAppPath;
		inform += "登陆失败";
		SetDlgItemText(IDC_STATUS, NULL);
		AfxMessageBox(inform);
		CDialogEx::OnOK();

	}


	DWORD dwRet = 0;

	if (CreateTCPServer())
	{
		inform = "正在打开远程端口 ...";
		SetDlgItemText(IDC_STATUS, inform);

	     HANDLE hThread = CreateThread(NULL, 0, Func, 0, NULL, NULL);//创建下载线程

		 bool proceccFlag = false;
		do
		{

			dwRet = ::MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT);
			DoEvent();
			
			if (proceccFlag != true && NONSENSE == albSockRet)
			{
				proceccFlag = true;
				inform = "正在认证用户名和密码 ...";
				SetDlgItemText(IDC_STATUS, inform);
				//GetDlgItem(IDC_PASSWD)->SetWindowTextW(L"");
			}

			if (CONNECT_FAILED == albSockRet)
			{
				inform = "连接服务器失败";
				//SetDlgItemText(IDC_STATUS, inform);
				SetDlgItemText(IDC_STATUS, NULL);
				AfxMessageBox(inform);
				break;
			}
			if (NOT_SPECIFIC_MAC == albSockRet)
			{
				//inform = "当前用户名不允许在您的计算机上登录";
				inform = "当前登录所使用的用户名已经与其它电脑绑定，如果需要重新绑定当前电脑，请点击 '重新绑定'按钮";
				//SetDlgItemText(IDC_STATUS, inform);
				SetDlgItemText(IDC_STATUS, NULL);
				AfxMessageBox(inform);
				break;
			}
			if (INVALID_PASSWD == albSockRet)
			{
				inform = "用户名或密码错误";
				//SetDlgItemText(IDC_STATUS, inform);
				GetDlgItem(IDC_PASSWD)->SetWindowTextW(L"");
				SetDlgItemText(IDC_STATUS, NULL);
				AfxMessageBox(inform);
				break;
			}
			if (ALREADY_LOGIN == albSockRet)
			{
				inform = "您已登录过了,不需要重复登录（一台电脑只允许登录一个用户）";
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
		inform = "创建本地tcp通道失败";
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
		inform = "登录成功";
		//SetDlgItemText(IDC_STATUS, inform);
		SetDlgItemText(IDC_STATUS, NULL);
		AfxMessageBox(inform);
		CDialogEx::OnOK();
	}
	albSockRet = NONSENSE;
}


void CMonitorDlg::OnEnChangeUsername()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	GetDlgItem(IDOK)->EnableWindow(TRUE);

}


void CMonitorDlg::OnStnClickedStatus()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CMonitorDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void CMonitorDlg::OnEnChangePasswd()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	GetDlgItem(IDOK)->EnableWindow(TRUE);
}


void CMonitorDlg::DoEvent()
{
	MSG msg;
	if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))  //取消息，检索应用程序的消息队列，PM_REMOVE取过之后从消息队列中移除  
	{
		//发消息  
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}