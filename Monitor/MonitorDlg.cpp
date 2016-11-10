
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
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PROGRESS1, &CMonitorDlg::OnNMCustomdrawProgress1)
	ON_BN_CLICKED(IDOK, &CMonitorDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_USERNAME, &CMonitorDlg::OnEnChangeUsername)
	ON_STN_CLICKED(IDC_STATUS, &CMonitorDlg::OnStnClickedStatus)
	ON_BN_CLICKED(IDCANCEL, &CMonitorDlg::OnBnClickedCancel)
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



enum ALB_SOCK_RET
{

	CONNECT_FAILED = 1,
	CONNECT_SUCCESS,
	USERNAME_NOT_EXIST,
	INVALID_PASSWD,
};


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

static BOOL ClsePipe()
{
	if (!isNamePipeStarted)
	{
		return TRUE;
	}

	if (NULL != m_hPipe)
	{
		DisconnectNamedPipe(m_hPipe);
	}

	return TRUE;
}

static BOOL TellCLient(LPCSTR Buffer, DWORD BufSize)
{
	if (ConnectNamedPipe(m_hPipe, NULL) == FALSE) {
		CloseHandle(m_hPipe);
	}

	return WriteFile(m_hPipe, Buffer, BufSize, NULL, NULL);
}

static BOOL RecvClient(PNZCH Buffer, DWORD* ReadSize)
{
	return ReadFile(m_hPipe, Buffer, 1024, ReadSize, NULL);
}

void CMonitorDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CString			s_name, s_pass;
	//ALB_SOCK_RET	ret;
	CString			inform;
	//char			Buf[1024];

	UpdateData(TRUE);

	username.GetWindowText(s_name);
	passwd.GetWindowText(s_pass);

	//if (!isNamePipeStarted && !InitNamePipe())
	//{
	//	goto _ERREND;
	//}
	//
	

	char cWinDir[MAX_PATH];
	char cmd[MAX_PATH];
	const char *cname;
	const char *cpass;

	_bstr_t fuckName(s_name.GetBuffer());
	cname = fuckName;

	_bstr_t fuckPass(s_pass);
	cpass = fuckPass;

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
		0, //CREATE_NO_WINDOW,
		NULL,
		NULL, 
		&StartupInfo,
		&pi))
	{

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		inform = sAppPath;
		inform += " Failed";
		AfxMessageBox(inform);
	}
	inform = cmd;
	SetDlgItemText(IDC_STATUS, inform);

	////ret = CONNECT_SUCCESS;
	//ret = CONNECT_SUCCESS;

	//if (CONNECT_FAILED == ret)
	//{
	//	inform = "连接服务器失败 ...";
	//	SetDlgItemText(IDC_STATUS, inform);
	//}

	//if (CONNECT_SUCCESS == ret)
	//{
	//	inform = "正在验证用户名和密码 ...";
	//	SetDlgItemText(IDC_STATUS, inform);
	//}


	//if (USERNAME_NOT_EXIST == ret)
	//{
	//	inform = "用户名不存在 ...";
	//	SetDlgItemText(IDC_STATUS, inform);
	//}

	//if (INVALID_PASSWD == ret)
	//{
	//	inform = "密码错误 ...";
	//	SetDlgItemText(IDC_STATUS, inform);
	//}

	//inform = "登录失败";
	//SetDlgItemText(IDC_STATUS, inform);

}


void CMonitorDlg::OnEnChangeUsername()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
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
