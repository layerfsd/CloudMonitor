
// MonitorDlg.cpp : ʵ���ļ�
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



enum ALB_SOCK_RET
{

	CONNECT_FAILED = 10,
	CONNECT_SUCCESS,
	USERNAME_NOT_EXIST,
	INVALID_PASSWD,
	ALREADY_LOGIN,
};


static BOOL isNamePipeStarted = FALSE;
static HANDLE m_hPipe = NULL;

static BOOL InitNamePipe()
{
	// ��ֹ��γ�ʼ��
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


//���������ܵ�
bool CreateNamedPipeInServer();

//�������ܵ��ж�ȡ����
int NamedPipeReadInServer();



bool CreateNamedPipeInServer()
{

	//������Ҫ���������ܵ�
	//���ﴴ������˫��ģʽ��ʹ���ص�ģʽ�������ܵ�
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
	//����¼��Եȴ��ͻ������������ܵ�
	//���¼�Ϊ�ֶ������¼����ҳ�ʼ��״̬Ϊ���ź�״̬
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!hEvent)
	{
		return 1;
	}

	memset(&ovlpd, 0, sizeof(OVERLAPPED));

	//���ֶ������¼����ݸ� ovlap ����
	ovlpd.hEvent = hEvent;

	//�ȴ��ͻ�������
	if (!ConnectNamedPipe(hNamedPipe, &ovlpd))
	{
		if (ERROR_IO_PENDING != GetLastError())
		{
			CloseHandle(hNamedPipe);
			CloseHandle(hEvent);
			return 2;
		}
	}

	//�ȴ��¼� hEvent ʧ��
	if (WAIT_FAILED == WaitForSingleObject(hEvent, INFINITE))
	{
		CloseHandle(hNamedPipe);
		CloseHandle(hEvent);
		return 3;
	}
 
	int				 RetValue = 0;

	//�������ܵ��ж�ȡ����
	if (!ReadFile(hNamedPipe, &RetValue, sizeof(int), NULL, NULL))
	{
		return 4;
	}
	CloseHandle(hNamedPipe);
	CloseHandle(hEvent);

	return RetValue;
}

void CMonitorDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString			s_name, s_pass;
	//ALB_SOCK_RET	ret;
	CString			inform;
	//char			Buf[1024];

	UpdateData(TRUE);

	username.GetWindowText(s_name);
	passwd.GetWindowText(s_pass);

	if (s_name.GetLength() <= 0)
	{
		inform = "�û���������Ϊ��";
		SetDlgItemText(IDC_STATUS, inform);
		return;
	}

	if (s_pass.GetLength() <= 0)
	{
		inform = "���벻��Ϊ��";
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

		inform = "�����в�������ڿո� ...";
		SetDlgItemText(IDC_STATUS, inform);
		return;
	}

	memset(cmd, 0, MAX_PATH);
	GetCurrentDirectoryA(MAX_PATH, cWinDir);

	char* sAppPath = strcat(cWinDir, "\\CloudMonitor.exe");
	sprintf(cmd, "%s %s %s", sAppPath, cname, cpass);


	STARTUPINFOA   StartupInfo;//���������������Ϣ�ṹ����    
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
		CREATE_NO_WINDOW,
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
		CDialogEx::OnOK();

	}
	inform = cmd;
	SetDlgItemText(IDC_STATUS, inform);

	inform = "���ڵ�¼ ...";
	SetDlgItemText(IDC_STATUS, inform);


	if (CreateNamedPipeInServer())
	{
		inform = "OK ...";
		SetDlgItemText(IDC_STATUS, inform);

		int ret = NamedPipeReadInServer();

		if (CONNECT_FAILED == ret)
		{
			inform = "���ӷ�����ʧ��,������������";
			SetDlgItemText(IDC_STATUS, inform);
		}

		if (CONNECT_SUCCESS == ret)
		{
			inform = "��¼�ɹ� ...";
			SetDlgItemText(IDC_STATUS, inform);
			inform = "��¼�ɹ�";
			AfxMessageBox(inform);
			CDialogEx::OnOK();
		}


		if (USERNAME_NOT_EXIST == ret)
		{
			inform = "�û��������� ...";
			SetDlgItemText(IDC_STATUS, inform);
		}

		if (INVALID_PASSWD == ret)
		{
			inform = "������� ...";
			SetDlgItemText(IDC_STATUS, inform);
		}

		if (ALREADY_LOGIN == ret)
		{
			inform = "���ѵ�¼����,����Ҫ�ظ���¼.";
			AfxMessageBox(inform);
			CDialogEx::OnOK();
		}
	}
	else
	{
		inform = "Failed.";
		SetDlgItemText(IDC_STATUS, inform);
	}


	//CDialogEx::OnOK();
}


void CMonitorDlg::OnEnChangeUsername()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
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
}
