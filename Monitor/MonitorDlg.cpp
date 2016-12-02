
// MonitorDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Monitor.h"
#include "MonitorDlg.h"
#include "afxdialogex.h"

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


static BOOL isNamePipeStarted = FALSE;
static HANDLE m_hPipe = NULL;

static BOOL InitNamePipe()
{
	// 防止多次初始化
	if (isNamePipeStarted)
	{
		return TRUE;
	}
	m_hPipe = CreateNamedPipeA("\\.\\Pipe\\LoginPipe", \
		PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 1, 0, 0, 1000, NULL);

	if (m_hPipe == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	

	return TRUE;
}

HANDLE            hNamedPipe;

const char *    pStr = "Zachary";
const char *    pPipeName = "\\\\.\\pipe\\ZacharyPipe";
HANDLE                    hEvent;
OVERLAPPED                ovlpd;


//创建命名管道
bool CreateNamedPipeInServer();

//从命名管道中读取数据
int NamedPipeReadInServer();



bool CreateNamedPipeInServer()
{

	//首先需要创建命名管道
	//这里创建的是双向模式且使用重叠模式的命名管道
	hNamedPipe = CreateNamedPipeA(pPipeName,
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
		0, 1, 1024, 1024, 0, NULL);

	if (INVALID_HANDLE_VALUE == hNamedPipe)
	{
		hNamedPipe = NULL;
		return false;
	}
	return true;
}

int NamedPipeReadInServer()
{
	//添加事件以等待客户端连接命名管道
	//该事件为手动重置事件，且初始化状态为无信号状态
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!hEvent)
	{
		return 1;
	}

	memset(&ovlpd, 0, sizeof(OVERLAPPED));

	//将手动重置事件传递给 ovlap 参数
	ovlpd.hEvent = hEvent;

	//等待客户端连接
	if (!ConnectNamedPipe(hNamedPipe, &ovlpd))
	{
		if (ERROR_IO_PENDING != GetLastError())
		{
			CloseHandle(hNamedPipe);
			CloseHandle(hEvent);
			return 2;
		}
	}

	//等待事件 hEvent 失败
	if (WAIT_FAILED == WaitForSingleObject(hEvent, INFINITE))
	{
		CloseHandle(hNamedPipe);
		CloseHandle(hEvent);
		return 3;
	}
 
	int				 RetValue = 0;

	char recvBuf[4] = { 0 };


	//从命名管道中读取数据
	if (!ReadFile(hNamedPipe, recvBuf, 4, NULL, NULL))
	{
		return 4;
	}
	CloseHandle(hNamedPipe);
	CloseHandle(hEvent);

	RetValue = atoi(recvBuf);
	return RetValue;
}

static int  albSockRet;

DWORD WINAPI Func(void* pArg)
{
	albSockRet = NamedPipeReadInServer();
	return 0;
}

void CMonitorDlg::OnBnClickedOk()
{
	
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	SetDlgItemText(IDC_STATUS, L" ");

	// TODO: 在此添加控件通知处理程序代码
	CString			s_name, s_pass;
	//ALB_SOCK_RET	ret;
	CString			inform;
	//char			Buf[1024];

	UpdateData(TRUE);

	username.GetWindowText(s_name);
	passwd.GetWindowText(s_pass);

	if (s_name.GetLength() <= 0)
	{
		inform = "用户名不允许为空";
		SetDlgItemText(IDC_STATUS, inform);
		return;
	}

	if (s_pass.GetLength() <= 0)
	{
		inform = "密码不能为空";
		SetDlgItemText(IDC_STATUS, inform);
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

		inform = "密码中不允许存在空格 ...";
		SetDlgItemText(IDC_STATUS, inform);
		return;
	}

	memset(cmd, 0, MAX_PATH);
	GetCurrentDirectoryA(MAX_PATH, cWinDir);

	char* sAppPath = strcat(cWinDir, "\\CloudMonitor.exe");
	sprintf(cmd, "%s %s %s", sAppPath, cname, cpass);


	STARTUPINFOA   StartupInfo;//创建进程所需的信息结构变量    
	PROCESS_INFORMATION pi;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));

	StartupInfo.cb = sizeof(StartupInfo);

	// Start the child process

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
		inform += " 登陆失败";
		AfxMessageBox(inform);
		CDialogEx::OnOK();

	}


	DWORD dwRet = 0;

	if (CreateNamedPipeInServer())
	{
		inform = "正在打开远程端口 ...";
		SetDlgItemText(IDC_STATUS, inform);

		//CWinThread* pThread = NULL;
	     HANDLE hThread = CreateThread(NULL, 0, Func, 0, NULL, NULL);//创建下载线程
	//	pThread = AfxBeginThread(StartThread, (LPVOID)NULL);  //起线程  
		 bool proceccFlag = false;
		do
		{

			dwRet = ::MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT);
			DoEvent();
			
			if (proceccFlag != true && NONSENSE == albSockRet)
			{
				proceccFlag = true;
				inform = "正在认证用户名和密码";
				SetDlgItemText(IDC_STATUS, inform);
				//GetDlgItem(IDC_PASSWD)->SetWindowTextW(L"");
			}

			if (CONNECT_FAILED == albSockRet)
			{
				inform = "连接服务器失败";
				SetDlgItemText(IDC_STATUS, inform);
				//GetDlgItem(IDC_PASSWD)->SetWindowTextW(L"");
				break;
			}
			if (USERNAME_NOT_EXIST == albSockRet)
			{
				inform = "用户名不存在";
				SetDlgItemText(IDC_STATUS, inform);
				GetDlgItem(IDC_USERNAME)->SetWindowTextW(L"");
				GetDlgItem(IDC_PASSWD)->SetWindowTextW(L"");
				break;
			}
			if (INVALID_PASSWD == albSockRet)
			{
				inform = "用户名或密码错误！";
				SetDlgItemText(IDC_STATUS, inform);
				GetDlgItem(IDC_PASSWD)->SetWindowTextW(L"");
				AfxMessageBox(inform);
				break;
			}
			if (ALREADY_LOGIN == albSockRet)
			{
				//inform = "您已登录过了,不需要重复登录.";
				inform = "一台电脑只允许登录一个用户";
				SetDlgItemText(IDC_STATUS, inform);
				AfxMessageBox(inform);
				break;
			}
		} while ((dwRet != WAIT_OBJECT_0) && (dwRet != WAIT_FAILED));
		CloseHandle(hThread);
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	else
	{
		inform = "登陆失败.";
		SetDlgItemText(IDC_STATUS, inform);
		CDialogEx::OnOK();
	}

	if (ALREADY_LOGIN == albSockRet)
	{
		CDialogEx::OnOK();
	}
	if (CONNECT_SUCCESS == albSockRet)
	{
		inform = "登录成功";
		SetDlgItemText(IDC_STATUS, inform);
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

void CMonitorDlg::ThreadWork()
{
	albSockRet = (ALB_SOCK_RET)NamedPipeReadInServer();
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