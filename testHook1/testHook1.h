
// testHook1.h : testHook1 Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������


// CtestHook1App:
// �йش����ʵ�֣������ testHook1.cpp
//

class CtestHook1App : public CWinApp
{
public:
	CtestHook1App();


// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// ʵ��

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CtestHook1App theApp;
