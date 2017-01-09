
// MonitorDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"

enum ALB_SOCK_RET
{
	NONSENSE = 0,
	CONNECT_FAILED = 10,
	CONNECT_SUCCESS,
	USERNAME_NOT_EXIST,
	INVALID_PASSWD,
	ALREADY_LOGIN,
	NOT_SPECIFIC_MAC,
};

#define SERV_PORT	50005
#define DAEMON_APP	"Daemon.exe"

// CMonitorDlg �Ի���
class CMonitorDlg : public CDialogEx
{
// ����
public:
	CMonitorDlg(CWnd* pParent = NULL);	// ��׼���캯��

	void DoEvent();
	


// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MONITOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:

	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdrawProgress1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeUsername();
	CEdit username;
	CEdit passwd;
	afx_msg void OnStnClickedStatus();
	afx_msg void OnBnClickedCancel();
	CStatic inform;
	afx_msg void OnEnChangePasswd();

	afx_msg void OnBnClickedMfclink1();
	afx_msg void OnBnClickedMfclink2();
};
